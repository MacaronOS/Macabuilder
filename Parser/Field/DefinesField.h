#pragma once

#include <string>
#include <unordered_map>

class DefinesField {
public:
    DefinesField() = default;
    inline bool add_define(const std::string& key, const std::string& value)
    {
        if (m_defines.contains(&key)) [[unlikely]] {
            return false;
        }
        m_defines[&key] = &value;
        return true;
    }
    const std::unordered_map<std::string const*, std::string const*>& defines() const { return m_defines; }

private:
    std::unordered_map<std::string const*, std::string const*> m_defines;
};
