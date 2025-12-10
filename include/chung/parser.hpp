#pragma once

#include "chung/ast.hpp"
#include "chung/context.hpp"
#include "chung/error.hpp"

#define VALIDATE_TOKEN(token_, type, condition)                                                                        \
    if (current_token().type == TokenType::EOF ||) {                                                                   \
        return false;                                                                                                  \
    }                                                                                                                  \
    return current_token().type == type && conditional;

class ParseException : public Exception {
public:
    std::string exception_message;
    Token token;

    const std::string& source_line;

    ParseException(std::string exception_message, Token token, const std::string& source_line);
    std::string write(const std::vector<std::string>& source_lines) override;
};

class Parser {
public:
    Parser(const std::vector<Token> tokens, const std::vector<std::string> source_lines, Context& ctx);

    Token current_token() {
        if (tokens_idx >= tokens.size()) {
            return tokens[tokens.size() - 1];
        }
        return tokens[tokens_idx];
    }

    Token previous_token() {
        if (tokens_idx == 0) {
            return tokens[0];
        }
        return tokens[tokens_idx - 1];
    }

    Token next_token() {
        if (tokens_idx >= tokens.size()) {
            return tokens[tokens.size() - 1];
        }
        return tokens[tokens_idx + 1];
    }

    Token eat_token() {
        if (tokens_idx >= tokens.size()) {
            return tokens[tokens.size() - 1];
        }
        return tokens[tokens_idx++];
    }

    // inline void eat_token_until(std::vector<Token>& tokens) {
    //     while (std::find(tokens.begin(), tokens.end(), eat_token()) != tokens.end()) {}
    // }

    ParseException push_exception(const std::string& exception_message, const Token& token) {
        ParseException exception{exception_message, token, source_lines[token.loc.line - 1]};
        exceptions.push_back(exception);
        return exception;
    }

    std::vector<ParseException> get_exceptions() {
        return exceptions;
    }

    void match_simple(TokenType type, const std::string& exception_str) {
        if (current_token().type != type) {
            throw push_exception(exception_str, current_token());
        }
        eat_token();
    }

    void synchronize();

    std::unique_ptr<ExprAST> parse_call();
    std::unique_ptr<ExprAST> parse_identifier();
    std::unique_ptr<ExprAST> parse_parentheses();
    std::unique_ptr<ExprAST> parse_bin_op(int min_op_precedence, std::unique_ptr<ExprAST> lhs);
    std::unique_ptr<ExprAST> parse_primitive();
    std::unique_ptr<ExprAST> parse_primary();

    // Statements
    std::unique_ptr<BlockAST> parse_block();
    std::unique_ptr<StmtAST> parse_var_declaration();
    std::unique_ptr<StmtAST> parse_function();
    std::unique_ptr<StmtAST> parse_omg();
    std::unique_ptr<StmtAST> parse_expression_statement(bool require_semicolons);

    std::unique_ptr<ExprAST> parse_if_expr();

    // Heheheha
    std::unique_ptr<ExprAST> parse_expression();
    std::unique_ptr<StmtAST> parse_statement();

    // For now
    std::vector<std::unique_ptr<StmtAST>> parse();

private:
    std::vector<Token> tokens;
    std::vector<std::string> source_lines;
    Context& ctx;

    std::vector<ParseException> exceptions;
    size_t tokens_idx;
};
