#include "chung/stringify.hpp"
#include "chung/ast.hpp"
#include "chung/token.hpp"

#include <map>

const std::string indent_prefix = "├── ";

inline std::string indent(size_t indent_level) {
    std::string indentation;
    for (size_t i = 0; i < indent_level; i++) {
        indentation += "│   ";
    }

    return indentation;
}

std::string indent_string(size_t indent_level, const std::string& string) {
    return '\n' + indent(indent_level) + indent_prefix + string;
}

std::string stringify_op(const TokenType& op, bool verbose) {
    static const std::map<TokenType, std::pair<std::string, std::string>> token_to_string = {
        {TokenType::ADD, {"Add", "+"}},
        {TokenType::SUB, {"Subtract", "-"}},
        {TokenType::MUL, {"Multiply", "*"}},
        {TokenType::DIV, {"Divide", "/"}},
        {TokenType::MOD, {"Modulo", "%"}},
        {TokenType::POW, {"Power", "**"}},
        {TokenType::BITWISE_AND, {"BitwiseAnd", "&"}},
        {TokenType::BITWISE_OR, {"BitwiseOr", "|"}},
        {TokenType::BITWISE_NOT, {"BitwiseNot", "~"}},
        {TokenType::GREATER_EQUAL, {"GreaterEqual", ">="}},
        {TokenType::GREATER_THAN, {"GreaterThan", ">"}},
        {TokenType::LESS_EQUAL, {"LessEqual", "<="}},
        {TokenType::LESS_THAN, {"LessThan", "<"}},
        {TokenType::EQUAL, {"Equal", "=="}},
        {TokenType::ASSIGN, {"Assign", "="}}};

    if (verbose) {
        return token_to_string.at(op).first;
    }
    return token_to_string.at(op).second;
}

std::string stringify_symbol(const TokenType& symbol, bool verbose) {
    static const std::map<TokenType, std::pair<std::string, std::string>> token_to_string = {
        {TokenType::OPEN_PARENTHESES, {"OpenParentheses", "("}},
        {TokenType::CLOSE_PARENTHESES, {"CloseParentheses", ")"}},
        {TokenType::OPEN_BRACKETS, {"OpenBrackets", "["}},
        {TokenType::CLOSE_BRACKETS, {"CloseBrackets", "]"}},
        {TokenType::OPEN_BRACES, {"OpenBraces", "{"}},
        {TokenType::CLOSE_BRACES, {"CloseBraces", "}"}},
        {TokenType::DOT, {"Dot", "."}},
        {TokenType::COMMA, {"Comma", ","}},
        {TokenType::COLON, {"Colon", ":"}},
        {TokenType::SEMICOLON, {"Semicolon", ";"}},
        {TokenType::ARROW, {"Arrow", "->"}}};

    if (verbose) {
        return token_to_string.at(symbol).first;
    }
    return token_to_string.at(symbol).second;
}

std::string stringify_keyword(const TokenType& keyword) {
    static const std::map<TokenType, std::string> token_to_string = {
        {TokenType::FUNC, "Func"}, {TokenType::LET, "Let"},     {TokenType::IF, "If"},
        {TokenType::ELSE, "Else"}, {TokenType::__OMG, "__OMG"},
    };
    return token_to_string.at(keyword);
}

std::string stringify_type(const TokenType& type) {
    if (type == TokenType::EOF) {
        return "EndOfFile";
    } else if (type == TokenType::INVALID) {
        return "Invalid";
    } else if (type == TokenType::IDENTIFIER) {
        return "Identifier";
    } else if (is_operator(type)) {
        return "Operator";
    } else if (is_symbol(type)) {
        return "Symbol";
    } else if (is_keyword(type)) {
        return "Keyword";
    } else if (type == TokenType::INT64) {
        return "Int64";
    } else if (type == TokenType::UINT64) {
        return "UInt64";
    } else if (type == TokenType::FLOAT64) {
        return "Float64";
    } else {
        return "Unknown";
    }
}

std::string stringify(const Token& token) {
    if (token.type == TokenType::EOF) {
        return "EOF";
    } else if (token.type == TokenType::INVALID) {
        return "Invalid";
    } else if (is_operator(token.type)) {
        return stringify_op(token.type, false);
    } else if (is_symbol(token.type)) {
        return stringify_symbol(token.type, false);
    } else if (is_keyword(token.type)) {
        return stringify_keyword(token.type);
    } else if (token.type == TokenType::INT64 || token.type == TokenType::UINT64 || token.type == TokenType::FLOAT64 ||
               token.type == TokenType::STRING || token.type == TokenType::IDENTIFIER) {
        return token.text;
    } else {
        return "Unknown";
    }
}

std::string AST::stringify(size_t indent_level) {
    // OOF
    return indent(indent_level) + "Goofy ASF AST";
}

std::string StmtAST::stringify(size_t indent_level) {
    return indent(indent_level) + "Goofy statement";
}

std::string ExprAST::stringify(size_t indent_level) {
    return indent(indent_level) + "Goofy expression";
}

std::string BlockAST::stringify(size_t indent_level) {
    std::string string;
    for (auto& statement : body) {
        string += statement->stringify(indent_level);
    }

    return string;
}

std::string IfExprAST::stringify(size_t indent_level) {
    std::string string{indent_string(indent_level, "If Conditional:")};

    string += indent_string(indent_level + 1, "Condition:");
    string += condition->stringify(indent_level + 2);
    string += indent_string(indent_level + 1, "Body:");
    string += body->stringify(indent_level + 2);

    if (!else_body) {
        string += indent_string(indent_level + 1, "Else Body:");
        string += else_body->stringify(indent_level + 2);
    }

    return string;
}

std::string FunctionAST::stringify(size_t indent_level) {
    std::string string{indent_string(indent_level, "Function Declaration:")};

    string += indent_string(indent_level + 1, "Name: " + name);
    string += indent_string(indent_level + 1, "Parameters:");

    for (size_t i = 0; i < parameters.size(); i++) {
        string += indent_string(indent_level + 2, "Parameter " + std::to_string(i + 1) + ": " + parameters[i].name);
    }
    if (parameters.size() == 0) {
        string += indent_string(indent_level + 2, "No Parameters");
    }
    string += indent_string(indent_level + 1, "Return Type: " + type.name);

    string += indent_string(indent_level + 1, "Body:");
    string += body->stringify(indent_level + 2);

    return string;
}

std::string VarDeclareAST::stringify(size_t indent_level) {
    std::string string{indent_string(indent_level, "Variable Declaration:")};

    string += indent_string(indent_level + 1, "Name: " + name);
    string += indent_string(indent_level + 1, "Value:") + expr->stringify(indent_level + 2);

    return string;
}

std::string ParamDeclareAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Variable Declaration:"};

    string += "\n\t" + indentation + "Name: " + name;

    return string;
}

std::string OmgAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Secret OMG:"};

    string += '\n' + indentation + expr->stringify(indent_level + 1);

    return string;
}

std::string ExprStmtAST::stringify(size_t indent_level) {
    std::string string{indent_string(indent_level, "Expression Statement:")};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += expr->stringify(indent_level + 1);
    return string;
}

std::string BinaryExprAST::stringify(size_t indent_level) {
    std::string string{indent_string(indent_level, "Binary Operation:")};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += indent_string(indent_level + 1, "Operator: " + stringify_op(op, false));

    // 2 new indentation level: 1 for "Binary Operation" and another for the side
    string += indent_string(indent_level + 1, "Left Hand:") + lhs->stringify(indent_level + 2);
    string += indent_string(indent_level + 1, "Right Hand:") + rhs->stringify(indent_level + 2);

    return string;
}

std::string CallAST::stringify(size_t indent_level) {
    std::string string{indent_string(indent_level, "Function Call:")};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += indent_string(indent_level + 1, "Name: " + callee);
    string += indent_string(indent_level + 1, "Arguments:");

    for (size_t i = 0; i < arguments.size(); i++) {
        /*
        Function Call:
            Name: sigma
            Arguments:
                Argument 1:
                    sdgasg
                Argument 2:
                    skibidi
        */
        string += indent_string(indent_level + 2, "Argument " + std::to_string(i + 1) + ":");
        string += arguments[i]->stringify(indent_level + 3);
    }

    if (arguments.size() == 0) {
        string += indent_string(indent_level + 3, "No Arguments");
    }
    return string;
}

std::string PrimitiveAST::stringify(size_t indent_level) {
    // switch (value_type) {
    //     case ValueType::INT64:
    //         return indentation + "Int64: " + std::to_string(int64) + '\n';
    //     case ValueType::UINT64:
    //         return indentation + "UInt64: " + std::to_string(uint64) + '\n';
    //     case ValueType::FLOAT64:
    //         return indentation + "Float64: " + std::to_string(float64) + '\n';
    //     case ValueType::STRING:
    //         return indentation + "String: \"" + string + "\"\n";
    //     default:
    //         return indentation + "Invalid\n";
    // }
    return indent_string(indent_level, value);
}

std::string VariableAST::stringify(size_t indent_level) {
    return indent_string(indent_level, "Variable Name: " + name);
}
