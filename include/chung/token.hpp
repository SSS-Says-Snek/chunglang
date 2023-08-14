#pragma once

#include <iostream>
#include <vector>

#undef EOF

enum class TokenType {
    EOF, INVALID,

    IDENTIFIER,

    ADD, SUB, MUL, DIV, MOD, POW,
    BITWISE_AND, BITWISE_OR, BITWISE_NOT,
    ASSIGN,

    OPEN_PARENTHESES, CLOSE_PARENTHESES,
    OPEN_BRACKETS, CLOSE_BRACKETS,
    OPEN_BRACES, CLOSE_BRACES,
    ARROW,
    DOT, COMMA, COLON, SEMICOLON,

    DEF, LET, __OMG,

    // Primitives
    UINT64,
    INT64,
    FLOAT64,
    STRING
};

struct TokenVal {
    union {
        uint64_t uint64;
        int64_t int64;
        double float64;
    };

    // L unions (I'm lazy to write a tagged one)
    std::string identifier;
    std::u32string string;
};

struct Token {
    TokenType type;
    size_t beg;
    size_t end;

    size_t line_beg;
    size_t line_end;

    size_t line;
    size_t column;

    std::string text;

    Token(TokenType type, size_t beg, size_t end):
        type{type}, beg{beg}, end{end}, line{0}, column{0} {}
};

bool is_keyword(const std::string& identifier);

bool is_keyword(TokenType keyword);
bool is_symbol(TokenType symbol);
bool is_operator(TokenType op);