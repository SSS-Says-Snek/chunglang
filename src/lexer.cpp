#include <string>
#include <cwctype>
#include <iostream>
#include <sstream>
#include <vector>

#include "chung/lexer.hpp"
// #include "chung/utf.hpp"

#define HANDLE_SIMPLE(op_, op_name)                                                   \
    case op_name:                                                                 \
        tokens.push_back(make_token(op_, cursor, cursor + 1));                    \
        advance();                                                                \
        break;                                                                    \

#define HANDLE_ESCAPE_SEQUENCE(char_, actual_char) \
    case char_:                                    \
        string += actual_char;                     \
        advance();                                 \
        break;                                     \

inline bool is_identifier_char(char c) {
    if (std::isalpha(c) || c == '_') {
        return true;
    }
    return false;
}

LexException::LexException(const std::string& exception_message, size_t start, size_t end):
    exception_message{exception_message}, start{start}, end{end} {}

std::string LexException::write() {
    std::string string{"LexException at line " + std::to_string(line) + " column " + std::to_string(column) + ":\n"};
    string += '\t' + source_line + '\n';
    string += exception_message + '\n';
    
    return string;
}

Lexer::Lexer(const std::string& source): source{source}, source_lines{}, cursor{0} {
    size_t start = 0;
    size_t end = 0;

    while ((end = source.find('\n', start)) != std::string::npos) {
        source_lines.push_back(source.substr(start, end - start));
        start = end + 1;
    }
    source_lines.push_back(source.substr(start));
}

std::pair<std::vector<Token>, std::vector<LexException>> Lexer::lex() {
    std::vector<Token> tokens;
    std::vector<LexException> exceptions;
    
    while (true) {
        try { 
            // Skips whitespace
            while (std::iswspace(peek()) || peek() == '\n') {
                advance();
            }

            if (peek() == '\0') {
                tokens.push_back(Token{TokenType::EOF, cursor, cursor + 1});
                break;
            } else if (is_identifier_char(peek())) {
                size_t start = cursor;
                advance();

                // [a-zA-z0-9_]
                while (is_identifier_char(peek()) || std::iswdigit(peek())) {
                    advance();
                }

                std::string identifier = source.substr(start, cursor - start);
                TokenType type;

                if (is_keyword(identifier)) {
                    if (identifier == "def") {
                        type = TokenType::DEF;
                    } else if (identifier == "let") {
                        type = TokenType::LET;
                    } else if (identifier == "__omg") {
                        type = TokenType::__OMG;
                    }
                } else {
                    type = TokenType::IDENTIFIER;
                }

                tokens.push_back(make_token(type, start, cursor));
            } else if (std::isdigit(peek())) {
                size_t start = cursor;
                advance();
                while (std::iswdigit(peek())) {
                    advance();
                }

                char suffix = peek();
                std::string token_string = source.substr(start, cursor - start);
                TokenType type;

                switch (suffix) {
                    case 'U' | 'u': // Unsigned
                        try {
                            type = TokenType::UINT64;
                            advance();
                        } catch (...) {
                            // L
                            tokens.push_back(make_token(TokenType::INVALID, start, cursor));
                            throw LexException{"Value " + token_string + " too large to store in an uint64", start, cursor};
                        }
                        break;

                    case '.': { // Floating point
                        size_t decimal_start = cursor;
                        advance();
                        while (std::iswdigit(peek())) {
                            advance();
                        }
                        std::string float_string = token_string + source.substr(decimal_start, cursor - decimal_start);

                        try {
                            type = TokenType::FLOAT64;
                        } catch (...) {
                            tokens.push_back(make_token(TokenType::INVALID, start, cursor));
                            throw LexException{"Value " + float_string + " too large to store in an float64", start, cursor};
                        }
                        break;
                    }

                    default:
                        try {
                            type = TokenType::INT64;
                        } catch (...) {
                            // L
                            tokens.push_back(make_token(TokenType::INVALID, start, cursor));
                            throw LexException{"Value " + token_string + " too large to store in an int64", start, cursor};
                        }
                        break;
                }

                tokens.push_back(make_token(type, start, cursor));
            } else if (peek() == U'"') {
                std::string string;
                size_t start = cursor;
                advance();

                while (peek() != U'"') {
                    if (peek() == '\0') {
                        throw LexException{"Unterminated string", start, cursor};
                    }

                    if (peek() == U'\\') {
                        advance();
                        switch (peek()) {
                            HANDLE_ESCAPE_SEQUENCE(U'n', U'\n')
                            HANDLE_ESCAPE_SEQUENCE(U't', U'\t')
                            HANDLE_ESCAPE_SEQUENCE(U'r', U'\r')
                            HANDLE_ESCAPE_SEQUENCE(U'\"', U'\"')
                            HANDLE_ESCAPE_SEQUENCE(U'\'', U'\'')
                            HANDLE_ESCAPE_SEQUENCE(U'\\', U'\\')

                            // Goofy
                            HANDLE_ESCAPE_SEQUENCE(U'a', U'\a')
                            HANDLE_ESCAPE_SEQUENCE(U'b', U'\b')
                            HANDLE_ESCAPE_SEQUENCE(U'e', U'\e')
                            HANDLE_ESCAPE_SEQUENCE(U'f', U'\f')
                        }
                    } else {
                        string += advance();
                    }
                }
                advance();

                Token token{TokenType::STRING, start, cursor};
                token.text = std::move(string);
                tokens.push_back(token);
            } else {
                switch (peek()) {
                    case '-':
                        advance();
                        if (peek() == '>') { // Arrow (->)
                            advance();
                            tokens.push_back(make_token(TokenType::ARROW, cursor - 2, cursor));
                        } else { // Subtraction
                            tokens.push_back(make_token(TokenType::SUB, cursor - 1, cursor));
                        }
                        break;

                    case '/':
                        advance();
                        if (peek() == '/') { // Comment
                            advance();
                            while (peek() != '\0') {
                                if (advance() == '\n') {
                                    break;
                                }
                            }
                        } else { // Division
                            tokens.push_back(make_token(TokenType::DIV, cursor - 1, cursor));
                        }
                        break;

                    HANDLE_SIMPLE(TokenType::ADD, '+')
                    HANDLE_SIMPLE(TokenType::MUL, '*')
                    HANDLE_SIMPLE(TokenType::ASSIGN, '=')

                    HANDLE_SIMPLE(TokenType::OPEN_PARENTHESES, '(')
                    HANDLE_SIMPLE(TokenType::CLOSE_PARENTHESES, ')')
                    HANDLE_SIMPLE(TokenType::OPEN_BRACKETS, '[')
                    HANDLE_SIMPLE(TokenType::CLOSE_BRACKETS, ']')
                    HANDLE_SIMPLE(TokenType::OPEN_BRACES, '{')
                    HANDLE_SIMPLE(TokenType::CLOSE_BRACES, '}')

                    HANDLE_SIMPLE(TokenType::DOT, '.')
                    HANDLE_SIMPLE(TokenType::COMMA, ',')
                    HANDLE_SIMPLE(TokenType::COLON, ':')
                    HANDLE_SIMPLE(TokenType::SEMICOLON, ';')

                    default:
                        tokens.push_back(make_token(TokenType::INVALID, cursor, cursor + 1));
                        advance();
                }
            }
        } catch (LexException& exception) {
            // Compute lines and columns based on index of src
            size_t line = 1;
            size_t column = 1;
            for (size_t i = 0; i < exception.end; i++) {
                if (source[i] == '\n') {
                    column = 0;
                    line++;
                }
                column++;
            }
            exception.source_line = source_lines[line - 1];
            exception.line = line;
            exception.column = column;

            exceptions.push_back(exception);
        }
    }

    size_t line = 1;
    size_t column = 1;
    size_t token_idx = 0;
    size_t line_idx = 0;
    for (size_t i = 0; i <= source.length(); i++) {
        if (source[i] == '\n') {
            column = 0;
            line++;
            line_idx = i + 1;
        }
        
        if (tokens[token_idx].beg == i) {
            tokens[token_idx].line = line;
            tokens[token_idx].column = column - 1;

            tokens[token_idx].line_beg = i - line_idx;
            tokens[token_idx].line_end = tokens[token_idx].end - line_idx;
            // std::cout << stringify(tokens[token_idx].type) << " line: " << line << " column: " << column << '\n';
            token_idx++;
        }

        column++;
    }

    return std::make_pair(tokens, exceptions);
}
