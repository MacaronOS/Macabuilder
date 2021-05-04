#pragma once

#include <memory>
#include <vector>

class DefaultField {
public:
    DefaultField() = default;
    inline void add_command_to_sequence(const std::shared_ptr<std::string>& command)
    {
        m_default_sequence.push_back(command);
    }

private:
    std::vector<std::shared_ptr<std::string>> m_default_sequence {};
};
