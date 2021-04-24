#include "Context.h"
#include "Parser/Parser.h"

#include <iostream>

Context::Context(const std::string& path, Context::Operation operation)
    : m_path(path)
    , m_operation(operation)
{
}

void Context::run()
{
    auto parser = Parser(m_path, this);
    parser.run();

    std::cout << "include:\n";
    for (auto path : m_include.pats()) {
        std::cout << *path << " ";
    }

    std::cout << "\ndefines:\n";
    for (auto define : m_defines.defines()) {
        std::cout << *define.first << " : " << *define.second << "\n";
    }


}
