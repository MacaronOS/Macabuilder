#include "Parser.h"

#include <functional>
#include <iostream>

#include "../Context.h"

Parser::Parser(const std::string& path, Context* context)
    : context(context)
{
    lexer = Lexer(path);
}

Parser::Parser(Parser&& parser) noexcept
{
    *this = std::move(parser);
}

Parser& Parser::operator=(Parser&& parser) noexcept
{
    lexer = std::move(parser.lexer);
    m_tokens_idx = parser.m_tokens_idx;
    context = parser.context;
    parser.context = nullptr;
    return *this;
}

void Parser::run()
{
    lexer.run();

    while (auto token = lookup()) {
        if (token->type() != Token::Type::Default || !token->content_ptr()) {
            trigger_error_on_line(token->line(), "met unexpected token ");
        }
        if (token->content() == "Include") {
            parse_include();
            // as soon as include list is parsed, we are ready to process other files in different threads
            for (auto& path : context->m_include.paths()) {
                if (!context->run_as_childs(*path, Context::Operation::Parse)) {
                    trigger_error_on_line(token->line(), "Included path \"" + *path + "\" does not exist");
                }
            }
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
        context->m_include.add_path(token.content_ptr());
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
        context->m_defines.add_define(key.content(), value->content_ptr());
    });
}

void Parser::parse_commands()
{
    eat(); // Commands
    eat_sub_rule_hard();

    parse_line_by_line(1, [this](const Token& cmd) {
        eat_sub_rule_hard();
        parse_argument_list([&](const Token& token) {
            context->m_commands.append_to_command(cmd.content(), token.content_ptr());
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
                context->m_build.add_dependency(dependency.content_ptr());
            });
        } else if (build_subfield.content() == "Src") {
            eat_sub_rule_hard();
            parse_argument_list([&](const Token& source) {
                context->m_build.add_source(source.content_ptr());
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
                            if (!context->m_build.set_compiler_to_extension(extension.content_ptr(), compiler->content_ptr())) {
                                trigger_error_on_line(compiler->line(), "compiler redefinition");
                            }
                        } else if (compiler_or_flag->content() == "Flags") {
                            eat_sub_rule_hard();
                            parse_argument_list([&](const Token& flag) {
                                context->m_build.add_flag_to_extension(extension.content_ptr(), flag.content_ptr());
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
        } else if (build_subfield.content() == "Link") {
            eat_sub_rule_hard();

            parse_line_by_line(2, [this](const Token& linker_or_flags) {
                if (linker_or_flags.content() == "Linker") {
                    eat_sub_rule_hard();
                    auto linker = parse_single_argument_of_rule(linker_or_flags);
                    if (!linker) {
                        trigger_error_on_line(linker_or_flags.line(), "no linker is specified");
                    } else {
                        context->m_build.set_linker(linker->content_ptr());
                    }
                } else if (linker_or_flags.content() == "Flags") {
                    eat_sub_rule_hard();
                    parse_argument_list_of_rule(linker_or_flags, [this](const Token& flag) {
                        context->m_build.add_linker_flag(flag.content_ptr());
                    });
                } else {
                    trigger_error_on_line(linker_or_flags.line(), "unknown Link option \"" + linker_or_flags.content() + "\"");
                }
            });
        } else if (build_subfield.content() == "Archiver") {
            eat_sub_rule_hard();
            auto archiver = parse_single_argument_of_rule(build_subfield);
            if (!archiver) {
                trigger_error_on_line(build_subfield.line(), "no Archiver is specified");
            } else {
                context->m_build.set_archiver(archiver->content_ptr());
            }
        } else {
            trigger_error_on_line(build_subfield.line(), "met unexpected Build subfield \"" + build_subfield.content() + "\"");
        }
    });
}

void Parser::parse_default()
{
    eat(); // Default
    eat_sub_rule_hard();

    parse_argument_list([this](const Token& token) {
        context->m_default.add_command_to_sequence(token.content_ptr());
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
