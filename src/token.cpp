#include <algorithm>
#include <vector>

#include "chung/token.hpp"

bool is_keyword(const std::string& identifier) {
    static const std::vector<std::string> keyword_identifiers{"func", "let", "__omg", "return", "if", "else"};

    return std::find(std::begin(keyword_identifiers), std::end(keyword_identifiers), identifier) !=
           std::end(keyword_identifiers);
}

bool is_keyword(TokenType keyword) {
    static const std::vector<TokenType> keywords{TokenType::FUNC,   TokenType::LET, TokenType::__OMG,
                                                 TokenType::RETURN, TokenType::IF,  TokenType::ELSE};

    return std::find(std::begin(keywords), std::end(keywords), keyword) != std::end(keywords);
}

bool is_symbol(TokenType symbol) {
    static const std::vector<TokenType> symbols{TokenType::OPEN_PARENTHESES,
                                                TokenType::CLOSE_PARENTHESES,
                                                TokenType::OPEN_BRACKETS,
                                                TokenType::CLOSE_BRACKETS,
                                                TokenType::OPEN_BRACES,
                                                TokenType::CLOSE_BRACES,
                                                TokenType::ARROW,
                                                TokenType::DOT,
                                                TokenType::COMMA,
                                                TokenType::COLON,
                                                TokenType::SEMICOLON};

    return std::find(std::begin(symbols), std::end(symbols), symbol) != std::end(symbols);
}

bool is_operator(TokenType op) {
    static const std::vector<TokenType> ops{TokenType::ADD,         TokenType::SUB,           TokenType::MUL,
                                            TokenType::DIV,         TokenType::MOD,           TokenType::POW,
                                            TokenType::BITWISE_AND, TokenType::BITWISE_OR,    TokenType::BITWISE_NOT,
                                            TokenType::ASSIGN,      TokenType::GREATER_EQUAL, TokenType::GREATER_THAN,
                                            TokenType::LESS_EQUAL,  TokenType::LESS_THAN,     TokenType::EQUAL};

    return std::find(std::begin(ops), std::end(ops), op) != std::end(ops);
}
