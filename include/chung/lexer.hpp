#pragma once

#include <vector>

#include "chung/token.hpp"
#include "chung/error.hpp"

class LexException : public Exception {
public:
    std::string exception_message;

    size_t start;
    size_t end;
    size_t line;
    size_t column;

    std::string source_line;

    LexException(const std::string& exception_message, size_t start, size_t end);
    std::string write() override;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);

    char advance() {
        return source[cursor++];
    }

    char peek() {
        return source[cursor];
    }

    std::vector<std::string> get_source_lines() {
        return source_lines;
    }

    Token make_token(TokenType type, size_t beg, size_t end) {
        Token token{type, beg, end};
        token.text = source.substr(beg, end - beg);
        return token;
    }

    std::pair<std::vector<Token>, std::vector<LexException>> lex();

private:
    const std::string source;
    std::vector<std::string> source_lines;
    size_t cursor;
};
