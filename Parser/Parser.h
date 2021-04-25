#pragma once

#include "../Lexer/Lexer.h"
#include "../Lexer/Token.h"

#include <functional>
#include <iostream>
#include <vector>

#pragma once

using TokenProcessor = const std::function<void(const Token&)>&;

class Context;

class Parser {
public:
    Parser() = default;
    Parser(const std::string& path, Context* context);

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    Parser(Parser&&) noexcept;
    Parser& operator=(Parser&&) noexcept;

    void run();

private:
    void parse_include();
    void parse_defines();
    void parse_commands();
    void parse_build();
    void parse_default();

    void parse_line_by_line(size_t nesting, TokenProcessor);

    Token const* parse_single_argument();
    Token const* parse_single_argument_of_rule(const Token& rule);

    void parse_argument_list_of_rule(const Token& rule, TokenProcessor);
    void parse_argument_list(TokenProcessor);
    void parse_lined_argument_list(size_t line, TokenProcessor);

private:
    inline void eat_sub_rule_hard()
    {
        if (lookup()->type() != Token::Type::SubRule) {
            trigger_error_on_line(lookup()->line(), "SubRule expected");
        }
        eat(); // SubRule
    }

    Token const* lookup(int offset = 0)
    {
        if (m_tokens_idx + offset < lexer.tokens().size()) {
            return &lexer.tokens()[m_tokens_idx + offset];
        } else {
            return nullptr;
        }
    }

    Token const* eat()
    {
        auto token = lookup();
        if (token) {
            m_tokens_idx++;
        }
        return token;
    }

    static void trigger_error_on_line(size_t line, const std::string& error)
    {
        std::cout << "line " << line << ": " << error << std::endl;
        exit(1);
    }

private:
    Context* context {};
    Lexer lexer {};
    std::size_t m_tokens_idx {};
};
