/*
 * Context object is an internal representation of a Beegn file,
 * which is being processed by a thread.
 */

#pragma once

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
    Context(const std::string& path, Operation operation);

public:
    void run();

    Context* create_child(const std::string& path, Operation operation)
    {
        auto child = new Context(path, operation);
        m_childs.push_back(child);
        return child;
    }

public:
    bool done() const { return m_done; }

private:
    bool parse();
    bool build();

private:
    std::string m_path;
    Operation m_operation;

    IncludeField m_include {};
    DefinesField m_defines {};
    CommandsField m_commands {};
    BuildField m_build {};
    DefaultField m_default {};

    Parser parser {};

    std::thread* m_thread {};

    State m_state { State::NotStarted };
    bool m_done {};

    std::vector<Context*> m_childs {};
};
