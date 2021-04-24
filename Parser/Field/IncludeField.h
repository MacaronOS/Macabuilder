#pragma once

#include "../../Lexer/Token.h"

#include <string>
#include <vector>

class IncludeField {
public:
    IncludeField() = default;
    void add_path(const std::string& path) { m_paths.push_back(&path); }
    const std::vector<std::string const*>& pats() const { return m_paths; }

private:
    std::vector<std::string const*> m_paths {};
};
