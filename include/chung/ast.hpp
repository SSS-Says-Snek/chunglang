#pragma once

#include <llvm/IR/Value.h>
#include <memory>
#include <utility>
#include <vector>

#include "chung/token.hpp"
#include "chung/type.hpp"
#include "chung/resolved_ast.hpp"

class AST {
public:
    virtual ~AST() = default;
    virtual std::string stringify(size_t indent_level = 0) = 0;
};

class StmtAST : public AST {
public:
    SourceLocation loc;

    StmtAST(SourceLocation loc) : loc{loc} {
    }

    std::string stringify(size_t indent_level = 0) override = 0;
};

class ExprAST : public AST {
public:
    SourceLocation loc;

    ExprAST(SourceLocation loc) : loc{loc} {
    }

    std::string stringify(size_t indent_level = 0) override = 0;
};

class BlockAST : public StmtAST {
public: 
    std::vector<std::unique_ptr<StmtAST>> body;

    BlockAST(SourceLocation loc, std::vector<std::unique_ptr<StmtAST>> body): StmtAST(loc), body{std::move(body)} {}

    std::string stringify(size_t indent_level = 0) override;
}; 

class DeclAST : public StmtAST {
public: 
    SourceLocation loc;
    std::string name;
    Type type;

    DeclAST(SourceLocation loc, std::string name, Type type) : StmtAST(loc), name{std::move(name)}, type{std::move(type)} {
    }

    std::string stringify(size_t indent_level = 0) override = 0;
};

class VarDeclareAST : public DeclAST {
public:
    std::unique_ptr<ExprAST> expr;

    VarDeclareAST(SourceLocation loc, std::string name, Type type, std::unique_ptr<ExprAST> expr)
        : DeclAST(loc, std::move(name), std::move(type)), expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
};

class ParamDeclareAST : public DeclAST {
public:
    ParamDeclareAST(SourceLocation loc, std::string name, Type type)
        : DeclAST(loc, std::move(name), std::move(type)) {
    }

    std::string stringify(size_t indent_level = 0) override;
};

class FunctionAST : public DeclAST {
public:
    std::vector<ParamDeclareAST> parameters;
    std::unique_ptr<BlockAST> body;

    FunctionAST(SourceLocation loc, std::string name, std::vector<ParamDeclareAST> parameters, Type return_type,
                std::unique_ptr<BlockAST> body)
        : DeclAST(loc, std::move(name), std::move(return_type)), parameters{std::move(parameters)}, body{std::move(body)} {
    }

    std::string stringify(size_t indent_level = 0) override;
};

class OmgAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr;

    OmgAST(SourceLocation loc, std::unique_ptr<ExprAST> expr) : StmtAST(loc), expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
};

class ExprStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr;

    ExprStmtAST(SourceLocation loc, std::unique_ptr<ExprAST> expr) : StmtAST(loc), expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
};

class BinaryExprAST : public ExprAST {
public:
    TokenType op;
    std::unique_ptr<ExprAST> lhs;
    std::unique_ptr<ExprAST> rhs;

    BinaryExprAST(SourceLocation loc, TokenType op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : ExprAST(loc), op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {
    }

    std::string stringify(size_t indent_level) override;
};

class CallAST : public ExprAST {
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> arguments;

    CallAST(SourceLocation loc, std::string callee, std::vector<std::unique_ptr<ExprAST>> arguments)
        : ExprAST(loc), callee{std::move(callee)}, arguments{std::move(arguments)} {
    }

    std::string stringify(size_t indent_level) override;
};

class IfExprAST : public ExprAST {
public:
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<BlockAST> body;
    std::unique_ptr<BlockAST> else_body;

    IfExprAST(SourceLocation loc, std::unique_ptr<ExprAST> condition, std::unique_ptr<BlockAST> body,
              std::unique_ptr<BlockAST> else_body)
        : ExprAST(loc), condition{std::move(condition)}, body{std::move(body)}, else_body{std::move(else_body)} {
    }

    std::string stringify(size_t indent_level) override;
};

class PrimitiveAST : public ExprAST {
public:
    TokenType type;
    std::string value;

    PrimitiveAST(SourceLocation loc, TokenType type) : ExprAST(loc), type{type} {
    }

    PrimitiveAST(SourceLocation loc, TokenType type, std::string value) : ExprAST(loc), type{type}, value{std::move(value)} {
    }

    std::string stringify(size_t indent_level = 0) override;
};

class VariableAST : public ExprAST {
public:
    std::string name;

    VariableAST(SourceLocation loc, std::string name) : ExprAST(loc), name{std::move(name)} {
    }

    std::string stringify(size_t indent_level = 0) override;
};
