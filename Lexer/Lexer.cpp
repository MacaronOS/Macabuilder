#include "Lexer.h"

#include <iostream>

void Lexer::run()
{
    while (next_string()) {
        int nesting = calc_nesting_for_cur_line();
        std::string cur_word {};

        while (char c = eat()) {
            auto token = Token::TokenFromChar(c, nesting);
            if (token.type() == Token::Type::Default) {
                cur_word.push_back(c);
            } else {
                m_tokens.emplace_back(std::move(cur_word), Token::Type::Default, nesting);
                m_tokens.push_back(token);
                eat_spaces();
                cur_word = {};
            }
        }

        if (!cur_word.empty()) {
            m_tokens.emplace_back(cur_word, Token::Type::Default, nesting);
        }

    }

    for (const auto& token : m_tokens) {
        std::cout << token.to_string() << ", ";
    }

}
int Lexer::calc_nesting_for_cur_line()
{
    int nesting = 0;
    while (char cur = lookup()) {
        if (cur == ' ') {
            eat();
            if (chars_remain() < 3) {
                return nesting;
            }
            for (size_t i = 0; i < 3; i++) {
                if (lookup() != ' ') {
                    return nesting;
                }
                eat();
            }
            nesting++;
            continue;
        }

        if (cur == '\n') {
            nesting++;
            eat();
            continue;
        }

        return nesting;
    }

    return nesting;
}

bool Lexer::next_string()
{
    m_line_idx = 0;
    return bool(getline(m_stream, m_cur_line));
}

void Lexer::eat_spaces()
{
    while (char c = lookup()) {
        if (c != ' ') {
            return;
        }
        eat();
    }
}
