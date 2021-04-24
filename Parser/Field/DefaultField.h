#pragma once

#include <vector>

class DefaultField {
public:
    DefaultField() = default;
    inline void add_command_to_sequence(const std::string& command)
    {
        m_default_sequence.push_back(&command);
    }

private:
    std::vector<std::string const*> m_default_sequence {};
};
