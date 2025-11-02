#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>

#include "chung/parser.hpp"
#include "chung/utils/ansi.hpp"

#define MATCH_NO_SYNC(condition, exception_string)                                                                     \
    if (!(current_token().condition)) {                                                                                \
        push_exception(exception_string, current_token());                                                             \
    }                                                                                                                  \
    eat_token();

int get_op_precedence(TokenType op) {
    static const std::unordered_map<TokenType, int> op_lookup{{TokenType::ADD, 1}, {TokenType::SUB, 1},
                                                              {TokenType::MUL, 2}, {TokenType::DIV, 2},
                                                              {TokenType::MOD, 2}, {TokenType::POW, 3}};

    auto result = op_lookup.find(op);
    if (result == op_lookup.end()) {
        return -1;
    }
    return result->second;
}

ParseException::ParseException(std::string exception_message, Token token, std::string source_line)
    : exception_message{std::move(exception_message)}, token{std::move(token)}, source_line{std::move(source_line)} {
}

std::string ParseException::write(const std::vector<std::string>& source_lines) {
    std::string string{ANSI_RED};
    string += "ParseException at line " + std::to_string(token.line) + " column " +
                       std::to_string(token.column) + ":\n" + ANSI_RESET;
    std::string carets;

    for (size_t i = 0; i <= source_line.length(); i++) {
        if (i == token.line_beg) {
            carets += ANSI_RED;
        }
        if (i == token.line_end) {
            carets += ANSI_RESET;
        }

        if (token.line_beg <= i && i < token.line_end) {
            carets += '^';
        } else {
            carets += '~';
        }

    }

    if (token.line > 0) {
        string += "|\t" + source_lines[token.line - 1 - 1] + '\n';
    }

    string += "|\t";
    string += std::string{ANSI_RED} + source_line + ANSI_RESET + '\n';
    string += "|\t" + carets + '\n';

    if (token.line < source_lines.size()) {
        string += "|\t" + source_lines[token.line] + '\n';
    }
    string += ANSI_RED + exception_message + ANSI_RESET + '\n';

    return string;
}

Parser::Parser(std::vector<Token> tokens, std::vector<std::string> source_lines, Context& ctx)
    : tokens{std::move(tokens)}, source_lines{std::move(source_lines)}, ctx{ctx}, tokens_idx{0} {
}

void Parser::synchronize() {
    eat_token();

    while (current_token().type != TokenType::EOF) {
        Token prev = previous_token();
        if (prev.type == TokenType::SEMICOLON) {
            // std::cout << "Done synchronizing\n";
            // std::cout << stringify(next_token());
            return;
        } else {
            Token next = next_token();
            if (next.type == TokenType::LET) {
                // std::cout << "Done synchronizing\n";
                return;
            }
        }

        eat_token();
    }
}

std::unique_ptr<ExprAST> Parser::parse_call() {
    // Eat function callee
    Token callee = eat_token();

    // Eats '('
    match_simple(TokenType::OPEN_PARENTHESES, "Expected '(' after function callee");
    std::vector<std::unique_ptr<ExprAST>> arguments;

    bool running = true;
    while (running) {
        // Is there an arg? If so, parse and append. Otherwise just skip ts and go to the switch
        if (auto argument = parse_expression()) {
            arguments.push_back(std::move(argument));
        }

        switch (current_token().type) {
            case TokenType::CLOSE_PARENTHESES:
                running = false;
                break;
            case TokenType::COMMA:
                eat_token();
                break;
            default:
                throw push_exception("Expected ',' or ')' within function call", current_token());
        }
    }

    // Eat ')'
    eat_token();
    return std::make_unique<CallAST>(callee.text, std::move(arguments));
}

std::unique_ptr<ExprAST> Parser::parse_identifier() {
    Token token = current_token();
    Token next = next_token();

    if (next.type != TokenType::OPEN_PARENTHESES) {
        // Eat identifier
        eat_token();
        return std::make_unique<VariableAST>(token.text);
    }

    // A call
    return parse_call();
}

std::unique_ptr<ExprAST> Parser::parse_parentheses() {
    // Eat '('
    eat_token();
    std::unique_ptr<ExprAST> expr = parse_expression();
    if (!expr) {
        return nullptr;
    }

    // Eat ')'
    match_simple(TokenType::CLOSE_PARENTHESES, "Expected closing parenthesis ')'");
    return expr;
}

std::unique_ptr<ExprAST> Parser::parse_bin_op(int min_op_precedence, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        Token op = current_token();
        int op_precedence = get_op_precedence(op.type);

        if (op_precedence < min_op_precedence || !is_operator(op.type)) {
            return lhs;
        }
        // std::cout << "Binary op: " << stringify(op.value.op) << '\n';

        // Eat operator
        eat_token();
        std::unique_ptr<ExprAST> rhs = parse_primary();

        int next_op_precedence = get_op_precedence(current_token().type);
        if (op_precedence < next_op_precedence) {
            rhs = parse_bin_op(min_op_precedence + 1, std::move(rhs));
        }

        lhs = std::make_unique<BinaryExprAST>(op.type, std::move(lhs), std::move(rhs));
    }
}

std::unique_ptr<ExprAST> Parser::parse_primitive() {
    Token token = eat_token();

    switch (token.type) {
        case TokenType::INT64: {
            int64_t int64 = std::stoll(token.text);
            return std::make_unique<PrimitiveAST>(int64);
        }
        case TokenType::UINT64: {
            uint64_t uint64 = std::stoull(token.text);
            return std::make_unique<PrimitiveAST>(uint64);
        }
        case TokenType::FLOAT64: {
            double float64 = std::stod(token.text);
            return std::make_unique<PrimitiveAST>(float64);
        }
        case TokenType::STRING: {
            return std::make_unique<PrimitiveAST>(token.text);
        }
        default:
            // Invalid token
            throw push_exception("Invalid token in expression", token);
    }
}

std::unique_ptr<ExprAST> Parser::parse_primary() {
    Token token = current_token();
    if (token.type == TokenType::IDENTIFIER) {
        return parse_identifier();
    } else if (is_symbol(token.type)) {
        if (token.type == TokenType::OPEN_PARENTHESES) {
            return parse_parentheses();
        }
        return nullptr;
    } else {
        // std::cout << "Primitive\n";
        return parse_primitive();
    }
}

std::vector<std::unique_ptr<StmtAST>> Parser::parse_block() {
    // Eat '{'
    match_simple(TokenType::OPEN_BRACES, "Expected '{' at start of block");

    std::vector<std::unique_ptr<StmtAST>> statements;
    while (current_token().type != TokenType::CLOSE_BRACES) {
        if (current_token().type == TokenType::EOF) {
            throw push_exception("Expected '}', got EOF. You probably forgot to close the block", current_token());
        }

        // std::cout << "OOW" << stringify(current_token());
        statements.push_back(parse_statement());
        // std::cout << "WOW" << stringify(current_token());
    }

    // Eat '}'
    // std::cout << 'O' << stringify(eat_token());
    eat_token();

    return statements;
}

std::unique_ptr<StmtAST> Parser::parse_var_declaration() {
    // Eat 'let'
    eat_token();

    // Eat identifier
    Token identifier = current_token();
    if (identifier.type != TokenType::IDENTIFIER) {
        throw push_exception("Expected identifier to assign expression to", identifier);
    }
    eat_token();

    std::unique_ptr<ExprAST> expr = std::make_unique<PrimitiveAST>();
    if (current_token().type == TokenType::ASSIGN) {
        // Eat '='
        eat_token();

        expr = parse_expression();
    }
    if (!expr) {
        return nullptr;
    }

    match_simple(TokenType::SEMICOLON, "Expected ';' after identifier");

    // FOR NOW
    return std::make_unique<VarDeclareAST>(identifier.text, Type::tnone, std::move(expr));
}

std::unique_ptr<StmtAST> Parser::parse_function() {
    // Eat 'def'
    eat_token();

    // Get and eat function name
    Token name = current_token();
    match_simple(TokenType::IDENTIFIER, "Expected function name after 'def'");

    // Eat '('
    match_simple(TokenType::OPEN_PARENTHESES, "Expected '(' after function declaration");

    std::vector<VarDeclareAST> parameters;
    while (current_token().type != TokenType::CLOSE_PARENTHESES) {
        // Get and eat parameter name
        Token parameter = current_token();
        match_simple(TokenType::IDENTIFIER, "Expected parameter name in function declaration");

        // Eat ':'
        match_simple(TokenType::COLON, "Expected ':' after parameter name to specify parameter type");

        Token type_name = current_token();
        match_simple(TokenType::IDENTIFIER, "Expected type in parameter declaration");

        Type& type = ctx.get_type(type_name.text);
        if (type.ty == Ty::TINVALID) {
            throw push_exception("Type does not exist", type_name);
        }

        // No default values FOR NOW
        parameters.emplace_back(parameter.text, type, nullptr);

        switch (current_token().type) {
            case TokenType::COMMA:
                eat_token();
            case TokenType::CLOSE_PARENTHESES:
                break;
            default:
                throw push_exception("Expected either '(' or ',' in function parameter list", current_token());
        }
    }

    // Eat ')'
    match_simple(TokenType::CLOSE_PARENTHESES, "Expected ')' after parameter list");

    return std::make_unique<FunctionAST>(name.text, std::move(parameters), parse_block()); // parse_block() -> body
}

std::unique_ptr<StmtAST> Parser::parse_omg() {
    // Eat '__omg'
    eat_token();

    std::unique_ptr<ExprAST> expr = parse_expression();
    if (!expr) {
        return nullptr;
    }

    // Eat ';'
    match_simple(TokenType::SEMICOLON, "Expected ';' after value");

    return std::make_unique<OmgAST>(std::move(expr));
}

std::unique_ptr<ExprAST> Parser::parse_expression() {
    std::unique_ptr<ExprAST> lhs = parse_primary();
    if (lhs == nullptr) {
        return nullptr;
    }

    return parse_bin_op(0, std::move(lhs));
}

std::unique_ptr<StmtAST> Parser::parse_expression_statement() {
    std::unique_ptr<ExprAST> expr = parse_expression();

    // Eat ';'
    match_simple(TokenType::SEMICOLON, "Expected ';' after expression");

    if (!expr) {
        return nullptr;
    }

    return std::make_unique<ExprStmtAST>(std::move(expr));
}

std::unique_ptr<StmtAST> Parser::parse_statement() {
    try {
        Token token = current_token();
        if (is_keyword(token.type)) {
            switch (token.type) {
                case TokenType::LET:
                    return parse_var_declaration();
                case TokenType::DEF:
                    return parse_function();
                case TokenType::__OMG:
                    return parse_omg();
                default: {
                    std::cout << "You failed me.\n";
                    return nullptr;
                }
            }
        } else {
            return parse_expression_statement();
        }
    } catch (ParseException& exception) {
        synchronize();
        // std::cout << exception.write() << '\n';
        return nullptr;
    }
}

std::vector<std::unique_ptr<StmtAST>> Parser::parse() {
    std::vector<std::unique_ptr<StmtAST>> statements;

    while (current_token().type != TokenType::EOF) {
        std::unique_ptr<StmtAST> statement = parse_statement();
        if (statement) {
            statements.push_back(std::move(statement));
        }
    }

    return statements;
}
