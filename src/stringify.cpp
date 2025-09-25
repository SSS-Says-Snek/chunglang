#include "chung/stringify.hpp"

inline std::string indent(size_t indent_level) {
    std::string indentation;
    for (size_t i = 0; i < indent_level; i++) {
        indentation += "\t";
    }

    return indentation;
}

std::string stringify_op(const TokenType& op, bool verbose) {
    static const char* op_names[] = {
        "Add", "Subtract", "Multiply", "Divide", "Modulo", "Power",
        "BitwiseAnd", "BitwiseOr", "BitwiseNot",

        "Assign"
    };
    static const char *ops[] = {
        "+", "-", "*", "/", "%", "**",
        "&", "|", "~",
        "="
    };

    if (verbose) {
        return op_names[static_cast<int>(op) - 3];
    }
    return ops[static_cast<int>(op) - 3];
}

std::string stringify_symbol(const TokenType& symbol, bool verbose) {
    static const char* symbol_names[] = {
        "OpenParentheses", "CloseParentheses", "OpenBrackets", "CloseBrackets",
        "OpenBraces", "CloseBraces",
        "Dot", "Comma", "Colon", "Semicolon"
    };
    static const char* symbols[] = {
        "(", ")", "[", "]", "{", "}",
        "->",
        ".", ",", ":", ";"
    };

    if (verbose) {
        return symbol_names[static_cast<int>(symbol) - 13];
    }
    return std::string{symbols[static_cast<int>(symbol) - 13]};
}

std::string stringify_keyword(const TokenType& keyword) {
    static const char* keyword_names[] = {
        "Def", "Let", "__OMG"
    };
    return keyword_names[static_cast<int>(keyword) - 24];
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
    } else if (token.type == TokenType::IDENTIFIER) {
        return token.text;
    } else if (is_operator(token.type)) {
        return stringify_op(token.type, false);
    } else if (is_symbol(token.type)) {
        return stringify_symbol(token.type, false);
    } else if (is_keyword(token.type)) {
        return stringify_keyword(token.type);
    } else if (token.type == TokenType::INT64 || token.type == TokenType::UINT64 || token.type == TokenType::FLOAT64 || token.type == TokenType::STRING) {
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

std::string FunctionAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Function Declaration:"};

    string += "\n\t" + indentation + "Name: " + name;
    string += "\n\t" + indentation + "Parameters:";

    for (size_t i = 0; i < parameters.size(); i++) {
        string += "\n\t\t" + indentation + "Parameter " + std::to_string(i) + ": " + parameters[i].name;
    }
    if (parameters.size() == 0) {
        string += "\n\t\tNo Parameters";
    }

    return string;
}

std::string VarDeclareAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Variable Declaration:"};

    string += "\n\t" + indentation + "Name: " + name;
    string += "\n\t" + indentation + "Value:\n" + expr->stringify(indent_level + 2);
    
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
    
    string += '\n' + indentation + expr->stringify(indent_level + 1);
    return string;
}

std::string BinaryExprAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Binary Operation:"};

    string += "\n\t" + indentation + "Operator: " + stringify_type(op);

    // 2 new indentation level: 1 for "Binary Operation" and another for the side
    string += "\n\t" + indentation + "Left Hand:\n" + lhs->stringify(indent_level + 2);
    string += "\n\t" + indentation + "Right Hand:\n" + rhs->stringify(indent_level + 2);

    return string;
}

std::string CallAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);
    std::string string{indentation + "Call:"};

    string += "\n\t" + indentation + "Name: " + callee;
    string += "\n\t" + indentation + "Arguments:\n";
    
    for (size_t i = 0; i < arguments.size(); i++) {
        /*
        Call:
            Arguments:
                Argument 1:
                    sdgasg
        */
        string += indentation + "\t\tArgument " + std::to_string(i + 1) + ":\n" + arguments[i]->stringify(indent_level + 3);
    }

    return string;
}

std::string PrimitiveAST::stringify(size_t indent_level) {
    std::string indentation = indent(indent_level);

    switch (value_type) {
        case ValueType::INT64:
            return indentation + "Int64: " + std::to_string(int64) + '\n';
        case ValueType::UINT64:
            return indentation + "UInt64: " + std::to_string(uint64) + '\n';
        case ValueType::FLOAT64:
            return indentation + "Float64: " + std::to_string(float64) + '\n';
        case ValueType::STRING:
            return indentation + "String: \"" + string + "\"\n";
        default:
            return indentation + "Invalid\n";
    }
}

std::string VariableAST::stringify(size_t indent_level) {
    return "Amogus";
}