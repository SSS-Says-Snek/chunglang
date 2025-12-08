#include "chung/stringify.hpp"
#include "chung/ast.hpp"
#include "chung/token.hpp"

#include <map>

inline std::string indent(size_t indent_level) {
    std::string indentation;
    for (size_t i = 0; i < indent_level; i++) {
        indentation += "\t";
    }

    return indentation;
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
        {TokenType::FUNC, "Func"},   {TokenType::LET, "Let"},     {TokenType::IF, "If"},
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
        string += "\n" + statement->stringify(indent_level);
    }

    return string;
}

std::string IfExprAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "If Conditional:"};

    string += "\n\t" + indentation + "Condition:";
    string += "\n" + condition->stringify(indent_level + 2);
    string += "\n\t" + indentation + "Body:";
    string += body->stringify(indent_level + 2); // No '\n', BlockAST::stringify got us covered

    if (!else_body) {
        string += "\n\t" + indentation + "Else Body:";
        string += else_body->stringify(indent_level + 2);
    }

    return string;
}

std::string FunctionAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Function Declaration:"};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += "\n\t" + indentation + "Name: " + name;
    string += "\n\t" + indentation + "Parameters:";

    for (size_t i = 0; i < parameters.size(); i++) {
        string += "\n\t\t" + indentation + "Parameter " + std::to_string(i + 1) + ": " + parameters[i].name;
    }
    if (parameters.size() == 0) {
        string += "\n\t\t" + indentation + "No Parameters";
    }
    string += "\n\t" + indentation + "Return Type: " + type.name;

    string += "\n\t" + indentation + "Body:";
    string += body->stringify(indent_level + 2);

    return string;
}

std::string VarDeclareAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Variable Declaration:"};

    string += "\n\t" + indentation + "Name: " + name;
    string += "\n\t" + indentation + "Value:\n" + expr->stringify(indent_level + 2);

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
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Expression Statement:"};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += "\n" + expr->stringify(indent_level + 1);
    return string;
}

std::string BinaryExprAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Binary Operation:"};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += "\n\t" + indentation + "Operator: " + stringify_op(op, false);

    // 2 new indentation level: 1 for "Binary Operation" and another for the side
    string += "\n\t" + indentation + "Left Hand:\n" + lhs->stringify(indent_level + 2);
    string += "\n\t" + indentation + "Right Hand:\n" + rhs->stringify(indent_level + 2);

    return string;
}

std::string CallAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Function Call:"};

    // string += "\n\t" + indentation + "Indentation level: " + std::to_string(indent_level);
    string += "\n\t" + indentation + "Name: " + callee;
    string += "\n\t" + indentation + "Arguments:\n";

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
        string +=
            indentation + "\t\tArgument " + std::to_string(i + 1) + ":\n" + arguments[i]->stringify(indent_level + 3);
    }

    if (arguments.size() == 0) {
        string += indentation + "\t\tNo Arguments";
    }
    return string;
}

std::string PrimitiveAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);

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
    return indentation + value + '\n';
}

std::string VariableAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);

    return indentation + "Variable Name: " + name + '\n';
}
