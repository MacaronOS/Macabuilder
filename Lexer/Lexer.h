#pragma once
#include "Token.h"

#include <fstream>
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::ifstream& input)
        : m_stream(input)
    {
    }

    ~Lexer()
    {
        if (m_stream.is_open()) {
            m_stream.close();
        }
    }

    void run();

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
    std::ifstream& m_stream;
    std::size_t m_line_idx {};
    std::string m_cur_line {};

    std::vector<Token> m_tokens {};
};