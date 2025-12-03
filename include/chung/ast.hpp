#pragma once

#include <llvm/IR/Value.h>
#include <memory>
#include <utility>
#include <vector>

#include "chung/context.hpp"
#include "chung/token.hpp"
#include "chung/type.hpp"
#include "chung/resolved_ast.hpp"

class AST {
public:
    virtual ~AST() = default;
    virtual std::string stringify(size_t indent_level = 0) = 0;
    virtual llvm::Value* codegen(Context& ctx) = 0;

    virtual std::unique_ptr<ResolvedAST> resolve() = 0;
};

class StmtAST : public AST {
public:
    std::string stringify(size_t indent_level = 0) override = 0;
    llvm::Value* codegen(Context& ctx) override = 0;
    std::unique_ptr<ResolvedAST> resolve() override = 0;
};

class ExprAST : public AST {
public:
    std::string stringify(size_t indent_level = 0) override = 0;
    llvm::Value* codegen(Context& ctx) override = 0;
    std::unique_ptr<ResolvedAST> resolve() override = 0;
};

class VarDeclareAST : public StmtAST {
public:
    std::string name;
    Type& type;
    std::unique_ptr<ExprAST> expr;

    VarDeclareAST(std::string name, Type& type, std::unique_ptr<ExprAST> expr)
        : name{std::move(name)}, type{type}, expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class FunctionAST : public StmtAST {
public:
    std::string name;
    std::vector<VarDeclareAST> parameters;
    std::vector<std::unique_ptr<StmtAST>> body;

    // FOR NOW; I just want things to work
    std::unique_ptr<ExprAST> return_type;

    FunctionAST(std::string name, std::vector<VarDeclareAST> parameters, std::vector<std::unique_ptr<StmtAST>> body)
        : name{std::move(name)}, parameters{std::move(parameters)}, body{std::move(body)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class OmgAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr;

    OmgAST(std::unique_ptr<ExprAST> expr) : expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class ExprStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr;

    ExprStmtAST(std::unique_ptr<ExprAST> expr) : expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class BinaryExprAST : public ExprAST {
public:
    TokenType op;
    std::unique_ptr<ExprAST> lhs;
    std::unique_ptr<ExprAST> rhs;

    BinaryExprAST(TokenType op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {
    }

    std::string stringify(size_t indent_level) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class CallAST : public ExprAST {
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> arguments;

    CallAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> arguments)
        : callee{std::move(callee)}, arguments{std::move(arguments)} {
    }

    std::string stringify(size_t indent_level) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class IfExprAST : public ExprAST {
public:
    std::unique_ptr<ExprAST> condition;
    std::vector<std::unique_ptr<StmtAST>> body;
    std::vector<std::unique_ptr<StmtAST>> else_body;

    IfExprAST(std::unique_ptr<ExprAST> condition, std::vector<std::unique_ptr<StmtAST>> body,
              std::vector<std::unique_ptr<StmtAST>> else_body)
        : condition{std::move(condition)}, body{std::move(body)}, else_body{std::move(else_body)} {
    }

    std::string stringify(size_t indent_level) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class PrimitiveAST : public ExprAST {
public:
    std::string value;

    PrimitiveAST(std::string value) : value{std::move(value)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};

class VariableAST : public ExprAST {
public:
    std::string name;

    VariableAST(std::string name) : name{std::move(name)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
    std::unique_ptr<ResolvedAST> resolve() override;
};
