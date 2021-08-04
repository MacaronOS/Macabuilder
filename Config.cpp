#include "Config.h"

#include <string>

void Config::process_arguments(int argc, char** argv)
{
    if (argc > 0) {
        m_filename = std::string(argv[0]);
        m_arguments.reserve(argc);
        for (size_t at = 1; at < argc; at++) {
            m_arguments.emplace_back(argv[at]);
        }
    }

    if (m_arguments.empty()) {
        m_mode = Mode::Default;
        return;
    }

    if (m_arguments.size() == 1 && m_arguments[0] == "generate") {
        m_mode = Mode::Generate;
        return;
    }

    m_mode = Mode::CommandList;
}