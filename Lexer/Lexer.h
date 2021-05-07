#pragma once
#include "Token.h"

#include <fstream>
#include <string>
#include <vector>

class Lexer {
public:
    Lexer() = default;
    explicit Lexer(const std::string& path);
    ~Lexer();

    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    Lexer(Lexer&&) noexcept;
    Lexer& operator=(Lexer&&) noexcept;

public:
    void run();

public:
    const std::string path() const { return m_path; }
    const std::vector<Token>& tokens() const { return m_tokens; }

private:
    int calc_nesting_for_cur_line();
    bool next_string();
    void eat_spaces();

    inline char lookup() const
    {
        if (m_line_idx < m_cur_line.size()) {
            return m_cur_line[m_line_idx];
        }
        return 0;
    }

    inline char eat()
    {
        char c = lookup();
        if (c) {
            m_line_idx++;
        }
        return c;
    }

    inline size_t chars_remain() const
    {
        return m_cur_line.size() - m_line_idx;
    }

private:
    std::string m_path {};
    std::ifstream m_stream {};
    std::size_t m_line_idx {};
    std::string m_cur_line {};

    std::size_t m_token_line { 1 };
    std::vector<Token> m_tokens {};
};