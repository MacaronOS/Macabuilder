#include "Context.h"

#include "Executor/ExecutableUnit.h"
#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "Parser/Parser.h"

#include <numeric>
#include <thread>
#include <utility>

Context::Context(std::filesystem::path path, Context::Operation operation)
    : m_path(std::move(path))
    , m_operation(operation)
{
    parser = Parser(m_path, this);
}

void Context::run()
{
    m_thread = new std::thread([this]() {
        parser.run();
        validate_fields();
        merge_children();
        build();

        // wait for children
        for (auto child : m_children) {
            while (!child->done()) {
                std::this_thread::yield();
            }
        }

        m_done = true;
    });
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
            child->m_thread->join();
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

bool Context::build()
{
    std::vector<std::shared_ptr<std::string>> objects;

    for (auto& source : m_build.sources()) {
        auto files = Finder::FindFiles(directory(), *source);
        for (auto& file : files) {
            auto extension = file.string().substr(file.string().find_last_of('.') + 1);
            auto option = m_build.get_option_for_extension(extension);

            if (!option) {
                // TODO: trigger some error
                continue;
            }

            Finder::CreateDirectory("BeegnBuild" / Finder::GetFolder(file));

            auto object = std::make_shared<std::string>(("BeegnBuild" / file).string() + ".o");

            auto flags = option->flags;

            flags.push_back(std::make_shared<std::string>("-c"));
            flags.push_back(std::make_shared<std::string>(file));
            flags.push_back(std::make_shared<std::string>("-o"));
            flags.push_back(object);

            Executor::the().enqueue(std::make_shared<ExecutableUnit>(ExecutableUnit {
                .op = ::Operation::Compile,
                .ctx = this,
                .callee = option->compiler,
                .src = std::move(file),
                .binary = nullptr,
                .args = std::move(flags),
            }));

            objects.push_back(object);
        }
    }

    // wait for the compilation of all objects
    while (compile_counter > 0) {
        std::this_thread::yield();
    }

    // finalize objects
    if (!objects.empty()) {
        if (m_build.type() == BuildField::Type::StaticLib) {
            auto lib_name = std::make_shared<std::string>(("BeegnBuild" / directory() / directory().filename()).string() + ".a");

            auto archiver_flags = std::vector<std::shared_ptr<std::string>>();
            archiver_flags.push_back( std::make_shared<std::string>("rcs"));
            archiver_flags.push_back( lib_name);
            std::copy(objects.begin(), objects.end(), std::back_inserter(archiver_flags));

            Executor::the().enqueue(std::make_shared<ExecutableUnit>(ExecutableUnit {
                .op = ::Operation::Archive,
                .ctx = this,
                .callee = m_build.archiver(),
                .src = {},
                .binary = lib_name,
                .args = std::move(archiver_flags),
            }));
        } else {
            // add objects and the output binary path to the linker flags
            std::copy(objects.begin(), objects.end(), std::back_inserter(m_build.linker_flags()));
            m_build.linker_flags().push_back(std::make_shared<std::string>("-o"));
            auto link_exec = std::make_shared<std::string>("BeegnBuild" / directory() / directory().filename());
            m_build.linker_flags().push_back(link_exec);

            Executor::the().enqueue(std::make_shared<ExecutableUnit>(ExecutableUnit {
                .op = ::Operation::Link,
                .ctx = this,
                .callee = m_build.linker(),
                .src = {},
                .binary = link_exec,
                .args = std::move(m_build.linker_flags()),
            }));
        }

        while (!done_finalizer) {
            std::this_thread::yield();
        }
    }

    // make sure, that all the children are built
    for (auto child : m_children) {
        if (child->operation() == Context::Operation::Build && child->m_build.type() == BuildField::Type::Executable) {
            child->m_thread->join();
        }
    }

    return false;
}
