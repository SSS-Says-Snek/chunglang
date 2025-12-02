#pragma once

#include <cstdint>
#include <string>

#undef EOF

enum class TokenType : uint8_t {
    EOF,
    INVALID,

    IDENTIFIER,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_NOT,
    ASSIGN,

    GREATER_THAN,
    LESS_THAN,
    GREATER_EQUAL,
    LESS_EQUAL,
    EQUAL,

    OPEN_PARENTHESES,
    CLOSE_PARENTHESES,
    OPEN_BRACKETS,
    CLOSE_BRACKETS,
    OPEN_BRACES,
    CLOSE_BRACES,
    ARROW,
    DOT,
    COMMA,
    COLON,
    SEMICOLON,

    FUNC,
    LET,
    RETURN,
    IF,
    ELSE,
    __OMG,

    // Primitives
    UINT64,
    INT64,
    FLOAT64,
    STRING
};

struct SourceLocation {
    size_t line{};
    size_t column{};

    size_t token_length{};
};

struct Token {
    TokenType type;
    size_t beg;
    size_t end;

    SourceLocation loc;

    std::string text;

    Token(TokenType type, size_t beg, size_t end) : type{type}, beg{beg}, end{end} {
    }
};

bool is_keyword(const std::string& identifier);

bool is_keyword(TokenType keyword);
bool is_symbol(TokenType symbol);
bool is_operator(TokenType op);
