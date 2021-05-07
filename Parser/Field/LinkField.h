#pragma once

#include <memory>
#include <string>
#include <vector>

class LinkField {
public:
    LinkField() = default;

    auto& linker() { return m_linker; }
    auto& flags() { return m_flags; }

    void set_linker(const std::shared_ptr<std::string>& linker) { m_linker = linker; }
    void add_flag(const std::shared_ptr<std::string>& flag) { m_flags.push_back(flag); }

private:
    std::shared_ptr<std::string> m_linker;
    std::vector<std::shared_ptr<std::string>> m_flags;
};