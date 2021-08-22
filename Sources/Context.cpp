#include "Context.h"

#include "Config.h"
#include "Executor/ExecutableUnit.h"
#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "Parser/Parser.h"
#include "Translator/Translator.h"

#include <numeric>
#include <thread>
#include <utility>

std::unordered_map<std::string, Context*> Context::s_processing_contexts = {};
SpinLock Context::m_lock = {};

Context::Context(std::filesystem::path path, Context::Operation operation, bool root_ctx)
    : m_path(std::move(path))
    , m_operation(operation)
    , m_root_ctx(root_ctx)
{
    parser = Parser(m_path, this);
}

void Context::run()
{
    m_thread = new std::thread([this]() {
        parser.run();
        validate_fields();
        merge_children();
        process_by_mode();

        m_done = true;
    });
}

bool Context::run_as_childs(const std::string& pattern, Operation operation)
{
    auto maca_files = Finder::FindMacaFiles(directory(), pattern);
    if (maca_files.empty()) {
        return false;
    }
    for (auto& path : maca_files) {
        auto _ = ScopedLocker(m_lock);
        auto context = get_context_by_path(path);
        if (context == nullptr) {
            auto child = new Context(path, operation);
            m_children.push_back(child);
            register_context(path, child);
            child->run();
        } else {
            m_children.push_back(context);
        }
    }
    return true;
}

void Context::validate_fields()
{
    if (m_build.type() == BuildField::Type::Executable) {
        if (m_build.archiver()) {
            trigger_error("can\'t use Archiver subfield for Executable type");
        }
    } else if (m_build.type() == BuildField::Type::StaticLib) {
        if (m_build.linker() || !m_build.linker_flags().empty()) {
            trigger_error("can\'t use Link subfield for StaticLib type");
        }
    }
}

bool Context::merge_children()
{
    for (auto child : m_children) {
        if (child->operation() == Context::Operation::Parse) {
            while (child->m_state != Context::State::Parsed) {
                std::this_thread::yield();
            }
        }
    }

    for (auto child : m_children) {
        if (child->operation() == Context::Operation::Parse) {
            // merging Default field
            for (auto& define : child->m_defines.defines()) {
                m_defines.add_define(define.first, define.second);
            }

            // merging Commands field
            for (auto& command : child->m_commands.commands()) {
                m_commands.m_commands.emplace(command.first, command.second);
            }
        }
    }

    m_state = State::Parsed;
    return true;
}

void Context::process_by_mode()
{
    if (m_operation != Operation::Build) {
        return;
    }
    auto mode = Config::the().mode();
    const auto& arguments = Config::the().arguments();

    auto process_command = [&](const std::string& cmd) {
        // Build is a special command word that's reserved for unit building
        if (cmd == "Build") {
            build();
        } else {
            for (auto& command : m_commands.command_list(cmd)) {
                Executor::blocking_cmd(*command);
            }
        }
    };

    if (mode == Config::Mode::Default) {
        if (m_default.sequence().empty()) {
            process_command("Build");
        }
        for (auto& cmd : m_default.sequence()) {
            process_command(*cmd);
        }
        return;
    }

    if (mode == Config::Mode::CommandList) {
        for (auto& cmd : arguments) {
            process_command(cmd);
        }
        return;
    }

    if (mode == Config::Mode::Generate) {
        Translator::generate_cmake(this);
    }
}

bool Context::build()
{
    std::vector<std::shared_ptr<std::string>> objects {};

    for (auto& source : m_build.sources()) {
        auto files = Finder::FindFiles(directory(), *source);

        for (auto& file : files) {
            auto extension = file.string().substr(file.string().find_last_of('.') + 1);
            auto option = m_build.get_option_for_extension(extension);

            if (!option) {
                trigger_error("no option for extension \"" + extension + "\"");
            }

            auto object = (maca_path() / std::filesystem::proximate(file, directory())).string() + ".o";
            Finder::CreateDirectory(std::filesystem::path(object).parent_path());

            auto relative_source = std::filesystem::proximate(file, directory());
            auto relative_object = std::filesystem::proximate(object, directory());

            auto flags = option->flags;

            // TODO: fix this!
            if (*option->compiler != "nasm") {
                flags.push_back(std::make_shared<std::string>("-c"));
            }
            flags.push_back(std::make_shared<std::string>(relative_source));
            flags.push_back(std::make_shared<std::string>("-o"));
            flags.push_back(std::make_shared<std::string>(relative_object));

            Executor::the().enqueue(std::make_shared<ExecutableUnit>(ExecutableUnit {
                .op = ::Operation::Compile,
                .ctx = this,
                .callee = option->compiler,
                .src = std::move(file),
                .binary = nullptr,
                .args = std::move(flags),
                .cwd = cwd(),
            }));

            objects.push_back(std::make_shared<std::string>(relative_object));
        }
    }

    // wait for the compilation of all objects
    while (compile_counter > 0) {
        std::this_thread::yield();
    }

    // wait for the finalization of the dependent static libs
    auto dependency_libs = std::vector<std::shared_ptr<std::string>>();
    for (auto child : m_children) {
        if (child->operation() == Context::Operation::Build) {
            while (child->m_build.type() == BuildField::Type::Unknown) {
                std::this_thread::yield();
            }
            if (child->m_build.type() == BuildField::Type::StaticLib) {
                auto dependency_lib_relative = std::filesystem::proximate(child->static_library_path(), directory());
                dependency_libs.push_back(std::make_shared<std::string>(dependency_lib_relative));
                while (child->m_state != Context::State::Built) {
                    std::this_thread::yield();
                }
            }
        }
    }

    // finalize objects
    if (m_state == State::BuildError) {
        exit(1);
    }
    if (!objects.empty()) {
        if (m_build.type() == BuildField::Type::StaticLib) {
            size_t lastindex = m_path.string().find_last_of('.');
            std::string libname = m_path.string().substr(0, lastindex);

            auto lib = (maca_path() / std::filesystem::proximate(libname, directory())).string() + ".a";
            auto lib_relative = std::filesystem::proximate(lib, directory());
            auto lib_name = std::make_shared<std::string>(lib_relative);

            auto archiver_flags = std::vector<std::shared_ptr<std::string>>();
            archiver_flags.push_back(std::make_shared<std::string>("rcs"));
            archiver_flags.push_back(lib_name);
            std::copy(objects.begin(), objects.end(), std::back_inserter(archiver_flags));
            std::copy(dependency_libs.begin(), dependency_libs.end(), std::back_inserter(archiver_flags));

            Executor::the().enqueue(std::make_shared<ExecutableUnit>(ExecutableUnit {
                .op = ::Operation::Archive,
                .ctx = this,
                .callee = m_build.archiver(),
                .src = {},
                .binary = lib_name,
                .args = std::move(archiver_flags),
                .cwd = cwd() }));
        } else {
            //            m_build.linker_flags().push_back(std::make_shared<std::string>("-Wl,--start-group"));
            std::copy(dependency_libs.begin(), dependency_libs.end(), std::back_inserter(m_build.linker_flags()));
            std::copy(objects.begin(), objects.end(), std::back_inserter(m_build.linker_flags()));
            std::copy(dependency_libs.begin(), dependency_libs.end(), std::back_inserter(m_build.linker_flags()));
            std::copy(dependency_libs.begin(), dependency_libs.end(), std::back_inserter(m_build.linker_flags()));
            //            m_build.linker_flags().push_back(std::make_shared<std::string>("-Wl,--end-group"));

            m_build.linker_flags().push_back(std::make_shared<std::string>("-o"));
            auto link_exec = std::make_shared<std::string>(std::filesystem::proximate(executable_path(), directory()));
            m_build.linker_flags().push_back(link_exec);

            Executor::the().enqueue(std::make_shared<ExecutableUnit>(ExecutableUnit {
                .op = ::Operation::Link,
                .ctx = this,
                .callee = m_build.linker(),
                .src = {},
                .binary = link_exec,
                .args = std::move(m_build.linker_flags()),
                .cwd = cwd() }));
        }

        while (!done_finalizer) {
            std::this_thread::yield();
        }
    }

    if (m_state == State::BuildError) {
        exit(1);
    }

    // make sure, that all the children are built
    for (auto child : m_children) {
        if (child->operation() == Context::Operation::Build) {
            if (child->m_build.type() == BuildField::Type::Executable) {
                while (child->m_state != Context::State::Built) {
                    std::this_thread::yield();
                }
            }
        }
    }

    m_state = State::Built;

    if (root()) {
        Executor::the().stop();
        Executor::the().await();
    }

    return false;
}
