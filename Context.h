/*
 * Context object is an internal representation of a Beelder file,
 * which is being processed by a thread.
 */

#pragma once

#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "Logger.h"
#include "Parser/Parser.h"

#include "Parser/Field/BuildField.h"
#include "Parser/Field/CommandsField.h"
#include "Parser/Field/DefaultField.h"
#include "Parser/Field/DefinesField.h"
#include "Parser/Field/IncludeField.h"

#include <string>
#include <thread>
#include <unordered_map>

class Context {
    friend class Parser;
    friend class Executor;

public:
    enum class State {
        NotStarted,
        ParseError,
        Parsed,
        BuildError,
        Built,
    };

    enum class Operation {
        Parse,
        Build,
    };

public:
    Context(std::filesystem::path path, Operation operation);

public:
    void run();

    bool run_as_childs(const std::string& pattern, Operation operation)
    {
        auto beelder_files = Finder::FindBeelderFiles(directory(), pattern);
        if (beelder_files.empty()) {
            return false;
        }
        for (auto& path : beelder_files) {
            auto child = new Context(path, operation);
            m_children.push_back(child);
            child->run();
        }
        return true;
    }

public:
    inline bool done() const { return m_done; }
    inline Operation operation() const { return m_operation; };
    inline std::filesystem::path directory() const { return m_path.parent_path(); }
    inline std::string executable_path() const
    {
        auto filename = directory();
        if (filename.empty()) {
            filename = m_path.stem();
        }
        return "BeelderBuild" / directory() / filename;
    }
    inline std::string static_library_path() const { return executable_path() + ".a"; }

private:
    void validate_fields();
    bool merge_children();
    bool build();

private:
    inline void trigger_error(const std::string& error)
    {
        Log(Color::Red, m_path.string() + ":", error);
        exit(1);
    }

    static inline std::shared_ptr<std::string> shell()
    {
        static auto shell = std::make_shared<std::string>("/bin/bash");
        return shell;
    }

private:
    std::thread* m_thread {};
    std::filesystem::path m_path;
    Operation m_operation;
    State m_state { State::NotStarted };
    bool m_done {};

    // Parser
    Parser parser {};
    IncludeField m_include {};
    DefinesField m_defines {};
    CommandsField m_commands {};
    BuildField m_build {};
    DefaultField m_default {};

    // Executor
    std::atomic<int> compile_counter {};
    bool done_finalizer {};

    // childs options
    std::vector<Context*> m_children {};

    // if included context contains a build field it's going to be built separately
    std::vector<BuildField> m_children_builds {};
};
