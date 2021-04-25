#include "Context.h"
#include "Parser/Parser.h"

#include <iostream>
#include <thread>

Context::Context(const std::string& path, Context::Operation operation)
    : m_path(path)
    , m_operation(operation)
{
    parser = Parser(m_path, this);
}

void Context::run()
{
    m_thread = new std::thread([this]() {
        parser.run();

        std::cout << "include:\n";
        for (auto path : m_include.pats()) {
            std::cout << *path << " ";
        }

        std::cout << "\ndefines:\n";
        for (auto define : m_defines.defines()) {
            std::cout << *define.first << " : " << *define.second << "\n";
        }

        m_state = State::Parsed;

        // wait for children
        for (auto child : m_childs) {
            while (!child->done()) {
                std::this_thread::yield();
            }
        }

        m_done = true;
    });
}
