#pragma once
#include <memory>
#include <string>
#include <utility>

class Token {
public:
    enum class Type {
        Default,
        SubRule, // :
        Comma,
    };

public:
    Token(std::string content, Type type, int nesting, size_t line)
        : m_content(new std::string(std::move(content)))
        , m_type(type)
        , m_nesting(nesting)
        , m_line(line)
    {
    }

    Token(Type type, int nesting, size_t line)
        : m_type(type)
        , m_nesting(nesting)
        , m_line(line)
    {
    }

    static Token TokenFromChar(char c, int nesting, size_t line)
    {
        if (c == ',') {
            return Token(Type::Comma, nesting, line);
        }
        if (c == ':') {
            return Token(Type::SubRule, nesting, line);
        }
        return Token(Type::Default, nesting, line);
    }

    auto& content() { return *m_content; }
    const auto& content() const { return *m_content; }

    auto& content_ptr() { return m_content; }
    auto& content_ptr() const { return m_content; }

    Type type() const { return m_type; }
    int nesting() const { return m_nesting; }
    size_t line() const { return m_line; }

    std::string to_string() const
    {
        if (m_type == Type::Comma) {
            return "Comma";
        }
        if (m_type == Type::SubRule) {
            return "SubRule";
        }
        return "[" + content() + "]";
    }

private:
    std::shared_ptr<std::string> m_content {};
    Type m_type;
    int m_nesting;

    size_t m_line {};
};