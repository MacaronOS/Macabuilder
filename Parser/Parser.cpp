#include "Parser.h"

#include <functional>
#include <iostream>

#include "../Context.h"

Parser::Parser(const std::string& path, Context* context)
    : context(context)
{
    lexer = Lexer(path);
}

void Parser::run()
{
    std::cout << "Running Lexer" << std::endl;
    lexer.run();
    std::cout << "Running Parser" << std::endl;

    while (auto token = lookup()) {
        if (token->content() == "Include") {
            parse_include();
        } else if (token->content() == "Define") {
            parse_defines();
        } else if (token->content() == "Commands") {
            parse_commands();
        } else if (token->content() == "Build") {
            parse_build();
        } else if (token->content() == "Default") {
            parse_default();
        } else {
            trigger_error_on_line(token->line(), "met unexpected token " + token->to_string());
            break;
        }
    }
}

void Parser::parse_include()
{
    eat(); // Include
    eat_sub_rule_hard();

    parse_argument_list([this](const Token& token) {
        context->m_include.add_path(token.content());
    });
}

void Parser::parse_defines()
{
    eat(); // Define
    eat_sub_rule_hard();

    // define key - value pairs should be at separated lines
    // and have a nesting equals one
    parse_line_by_line(1, [this](const Token& key) {
        eat_sub_rule_hard();
        auto value = eat();
        if (!value || value->line() != key.line()) {
            trigger_error_on_line(key.line(), "invalid pair for a key");
        }
        context->m_defines.add_define(key.content(), value->content());
    });
}

void Parser::parse_commands()
{
    eat(); // Commands
    eat_sub_rule_hard();

    parse_line_by_line(1, [this](const Token& cmd) {
        eat_sub_rule_hard();
        parse_argument_list([&](const Token& token) {
            context->m_commands.append_to_command(cmd.content(), token.content());
        });
    });
}

void Parser::parse_build()
{
    eat(); // Build
    eat_sub_rule_hard();

    parse_line_by_line(1, [&](const Token& build_subfield) {
        if (build_subfield.content() == "Type") {
            eat_sub_rule_hard();
            auto type = parse_single_argument();
            if (!context->m_build.set_type(type->content())) {
                trigger_error_on_line(type->line(), "incorrect type (choose either StaticLib or Executable)");
            }
        } else if (build_subfield.content() == "Depends") {
            eat_sub_rule_hard();
            parse_argument_list([&](const Token& dependency) {
                context->m_build.add_dependency(dependency.content());
            });
        } else if (build_subfield.content() == "Src") {
            eat_sub_rule_hard();
            parse_argument_list([&](const Token& source) {
                context->m_build.add_source(source.content());
            });
        } else if (build_subfield.content() == "Extensions") {
            eat_sub_rule_hard();
            parse_line_by_line(2, [&](const Token& extension) {
                eat_sub_rule_hard();

                bool options_specified = false;

                for (size_t i = 0; i < 2; i++) {
                    auto compiler_or_flag = parse_single_argument_of_rule(extension);
                    if (compiler_or_flag) {
                        options_specified = true;
                        if (compiler_or_flag->content() == "Compiler") {
                            eat_sub_rule_hard();
                            auto compiler = parse_single_argument();
                            if (!compiler) {
                                trigger_error_on_line(compiler_or_flag->line(), " no compiler is specified");
                            }
                            if (!context->m_build.set_compiler_to_extension(extension.content(), compiler->content())) {
                                trigger_error_on_line(compiler->line(), "compiler redefinition");
                            }
                        } else if (compiler_or_flag->content() == "Flags") {
                            eat_sub_rule_hard();
                            parse_argument_list([&](const Token& flag) {
                                context->m_build.add_flag_to_extension(extension.content(), flag.content());
                            });
                        } else {
                            trigger_error_on_line(compiler_or_flag->line(), "invalid option for extension - " + compiler_or_flag->content());
                        }
                    }
                }

                if (!options_specified) {
                    trigger_error_on_line(extension.line(), "no options specified for extension");
                }
            });
        }
    });
}

void Parser::parse_default()
{
    eat(); // Default
    eat_sub_rule_hard();

    parse_argument_list([this](const Token& token) {
        context->m_default.add_command_to_sequence(token.content());
    });
}

void Parser::parse_line_by_line(size_t nesting, TokenProcessor process_token)
{
    while (lookup() && lookup(-1)->line() != lookup()->line() && lookup()->nesting() == nesting && lookup()->type() == Token::Type::Default) {
        process_token(*eat());
    }
}

Token const* Parser::parse_single_argument()
{
    Token const* arg = nullptr;
    parse_argument_list([&](const Token& token) {
        if (arg) [[unlikely]] {
            trigger_error_on_line(token.line(), "only one argument is allowed");
        }
        arg = &token;
    });
    return arg;
}

Token const* Parser::parse_single_argument_of_rule(const Token& rule)
{
    Token const* arg = nullptr;
    parse_argument_list_of_rule(rule, [&](const Token& token) {
        if (arg) [[unlikely]] {
            trigger_error_on_line(token.line(), "only one argument is allowed");
        }
        arg = &token;
    });
    return arg;
}

void Parser::parse_argument_list(TokenProcessor process_token)
{
    parse_argument_list_of_rule(*lookup(-1), process_token);
}

void Parser::parse_argument_list_of_rule(const Token& rule, TokenProcessor process_token)
{
    size_t sub_rule_line = rule.line();
    int sub_rule_nesting = rule.nesting();

    if (!lookup()) {
        return;
    }

    // one line argument list
    if (sub_rule_line == lookup()->line()) {
        parse_lined_argument_list(sub_rule_line, process_token);
        return;
    }

    // multiple lines argument list
    if (sub_rule_nesting < lookup()->nesting()) {
        int multiple_lines_nesting = lookup()->nesting();
        while (lookup() && lookup()->type() == Token::Type::Default && lookup()->nesting() == multiple_lines_nesting) {
            parse_lined_argument_list(lookup()->line(), process_token);
        }
    }
}

void Parser::parse_lined_argument_list(size_t line, TokenProcessor process_token)
{
    process_token(*eat());
    while (lookup() && lookup()->type() == Token::Type::Comma) {
        eat();
        if (lookup()->line() != line) {
            return;
        }
        if (lookup()->type() != Token::Type::Default) {
            return;
        }
        process_token(*eat());
    }
}