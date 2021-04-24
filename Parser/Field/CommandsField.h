#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class CommandsField {
public:
    CommandsField() = default;
    inline void append_to_command(const std::string& cmd, const std::string& append)
    {
        m_commands[&cmd].push_back(&append);
    }

    const std::unordered_map<std::string const*, std::vector<std::string const*>>& commands() const { return m_commands; }

private:
    std::unordered_map<std::string const*, std::vector<std::string const*>> m_commands {};
};
