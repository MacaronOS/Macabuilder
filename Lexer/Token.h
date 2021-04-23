#pragma once
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
    Token(std::string content, Type type, int nesting)
        : m_content(std::move(content))
        , m_type(type)
        , m_nesting(nesting)
    {
    }

    Token(Type type, int nesting)
        : m_type(type)
        , m_nesting(nesting)
    {
    }

    static Token TokenFromChar(char c, int nesting = 0)
    {
        if (c == ',') {
            return Token(Type::Comma, nesting);
        }
        if (c == ':') {
            return Token(Type::SubRule, nesting);
        }
        return Token(Type::Default, nesting);
    }

    std::string& content() { return m_content; }
    const std::string& content() const { return m_content; }

    Type type() const { return m_type; }

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
    std::string m_content;
    Type m_type;
    int m_nesting;
};