#pragma once

#include <string>
#include <unordered_map>

class DefinesField {
public:
    DefinesField() = default;
    inline bool add_define(const std::string& key, const std::shared_ptr<std::string>& value)
    {
        if (m_defines.contains(key)) [[unlikely]] {
            return false;
        }
        m_defines[key] = value;
        return true;
    }

    auto& defines() const { return m_defines; }

private:
    std::unordered_map<std::string, std::shared_ptr<std::string>> m_defines;
};
