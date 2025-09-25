#include <memory>
#include <vector>
#include <unordered_map>

#include "chung/parser.hpp"
#include "chung/stringify.hpp"

#define MATCH_NO_SYNC(condition, exception_string)                   \
    if (!(current_token().condition)) {                              \
        push_exception(exception_string, current_token());           \
    }                                                                \
    eat_token();                                                     \


int get_op_precedence(TokenType op) {
    static const std::unordered_map<TokenType, int> op_lookup {
        {TokenType::ADD, 1}, {TokenType::SUB, 1},
        {TokenType::MUL, 2}, {TokenType::DIV, 2}, {TokenType::MOD, 2},
        {TokenType::POW, 3}
    };

    auto result = op_lookup.find(op);
    if (result == op_lookup.end()) {
        return -1;
    }
    return result->second;
}


ParseException::ParseException(const std::string& exception_message, const Token& token, const std::string& source_line):
    exception_message{exception_message}, token{token}, source_line{source_line} {}

std::string ParseException::write() {
    std::string string{"ParseException at line " + std::to_string(token.line) + " column " + std::to_string(token.column) + ":\n"};
    std::string carets;

    for (size_t i = 0; i <= source_line.length(); i++) {
        if (token.line_beg <= i && i < token.line_end) {
            carets += '^';
        } else {
            carets += '~';
        }
    }

    string += '\t' + source_line + '\n';
    string += '\t' + carets + '\n';
    string += exception_message + '\n';
    
    return string;
}


Parser::Parser(const std::vector<Token> tokens, const std::vector<std::string> source_lines, Context& ctx):
    tokens{std::move(tokens)}, source_lines{std::move(source_lines)}, ctx{ctx}, tokens_idx{0} {}

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

std::shared_ptr<ExprAST> Parser::parse_call() {
    // Eat function callee
    Token callee = eat_token();

    // Eats '('
    match_simple(TokenType::OPEN_PARENTHESES, "Expected '(' after function callee");
    std::vector<std::shared_ptr<ExprAST>> arguments;

    bool running = true;
    while (running) {
        if (auto argument = parse_expression()) {
            arguments.push_back(argument);
        } else {
            return nullptr;
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
    return std::make_shared<CallAST>(callee.text, std::move(arguments));
}

std::shared_ptr<ExprAST> Parser::parse_identifier() {
    Token token = current_token();
    Token next = next_token();

    if (next.type != TokenType::OPEN_PARENTHESES) {
        // Eat identifier
        eat_token();
        return std::make_shared<VariableAST>(token.text);
    }

    // A call
    return parse_call();
}

std::shared_ptr<ExprAST> Parser::parse_parentheses() {
    // Eat '('
    eat_token();
    std::shared_ptr<ExprAST> expr = parse_expression();
    if (!expr) {
        return nullptr;
    }

    // Eat ')'
    match_simple(TokenType::CLOSE_PARENTHESES, "Expected closing parenthesis ')'");
    return expr;
}

std::shared_ptr<ExprAST> Parser::parse_bin_op(int min_op_precedence, std::shared_ptr<ExprAST> lhs) {
    while (true) {
        Token op = current_token();
        int op_precedence = get_op_precedence(op.type);

        if (op_precedence < min_op_precedence || !is_operator(op.type)) {
            return lhs;
        }
        // std::cout << "Binary op: " << stringify(op.value.op) << '\n';

        // Eat operator
        eat_token();
        std::shared_ptr<ExprAST> rhs = parse_primary();

        int next_op_precedence = get_op_precedence(current_token().type);
        if (op_precedence < next_op_precedence) {
            rhs = parse_bin_op(min_op_precedence + 1, std::move(rhs));
        }

        lhs = std::make_shared<BinaryExprAST>(op.type, std::move(lhs), std::move(rhs));
    }
}

std::shared_ptr<ExprAST> Parser::parse_primitive() {
    Token token = eat_token();

    switch (token.type) {
        case TokenType::INT64: {
            int64_t int64 = std::stoll(token.text);
            return std::make_shared<PrimitiveAST>(int64);
        }
        case TokenType::UINT64: {
            uint64_t uint64 = std::stoull(token.text);
            return std::make_shared<PrimitiveAST>(uint64);
        }
        case TokenType::FLOAT64: {
            double float64 = std::stod(token.text);
            return std::make_shared<PrimitiveAST>(float64);
        }
        case TokenType::STRING: {
            return std::make_shared<PrimitiveAST>(token.text);
        }
        default:
            // Invalid token
            throw push_exception("Invalid token in expression", token);
    }
}

std::shared_ptr<ExprAST> Parser::parse_primary() {
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

std::vector<std::shared_ptr<StmtAST>> Parser::parse_block() {
    // Eat '{'
    match_simple(TokenType::OPEN_BRACES, "Expected '{' at start of block");

    std::vector<std::shared_ptr<StmtAST>> statements;
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

std::shared_ptr<StmtAST> Parser::parse_var_declaration() {
    // Eat 'let'
    eat_token();

    // Eat identifier
    Token identifier = current_token();
    if (identifier.type != TokenType::IDENTIFIER) {
        throw push_exception("Expected identifier to assign expression to", identifier);
    }
    eat_token();
    
    std::shared_ptr<ExprAST> expr = std::make_shared<PrimitiveAST>();
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
    return std::make_shared<VarDeclareAST>(identifier.text, Type::tnone, std::move(expr));
}

std::shared_ptr<StmtAST> Parser::parse_function() {
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
        parameters.push_back(VarDeclareAST{parameter.text, type, nullptr});

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

    std::vector<std::shared_ptr<StmtAST>> body = parse_block();
    return std::make_shared<FunctionAST>(name.text, parameters, body);
}

std::shared_ptr<StmtAST> Parser::parse_omg() {
    // Eat '__omg'
    eat_token();

    std::shared_ptr<ExprAST> expr = parse_expression();
    if (!expr) {
        return nullptr;
    }

    // Eat ';'
    match_simple(TokenType::SEMICOLON, "Expected ';' after value");

    return std::make_shared<OmgAST>(expr);
}

std::shared_ptr<ExprAST> Parser::parse_expression() {
    std::shared_ptr<ExprAST> lhs = parse_primary();
    if (lhs == nullptr) {
        return nullptr;
    }

    return parse_bin_op(0, std::move(lhs));
}

std::shared_ptr<StmtAST> Parser::parse_expression_statement() {
    std::shared_ptr<ExprAST> expr = parse_expression();

    // Eat ';'
    match_simple(TokenType::SEMICOLON, "Expected ';' after expression");

    if (!expr) {
        return nullptr;
    }

    return std::make_shared<ExprStmtAST>(expr);
}

std::shared_ptr<StmtAST> Parser::parse_statement() {
    try {
        Token token = current_token();
        if (is_keyword(token.type)) {
            switch (token.type) {
                case TokenType::LET: return parse_var_declaration();
                case TokenType::DEF: return parse_function();
                case TokenType::__OMG: return parse_omg();
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

std::vector<std::shared_ptr<StmtAST>> Parser::parse() {
    std::vector<std::shared_ptr<StmtAST>> statements;

    while (current_token().type != TokenType::EOF) {
        std::shared_ptr<StmtAST> statement = parse_statement();
        if (statement) {
            statements.push_back(statement);
        }
    }

    return statements;
}