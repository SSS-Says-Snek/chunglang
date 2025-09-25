#pragma once

#include <memory>
#include <vector>

#include "chung/context.hpp"
#include "chung/token.hpp"
#include "chung/type.hpp"

class AST {
public:
    virtual ~AST() = default;
    virtual std::string stringify(size_t indent_level = 0) = 0;
    virtual llvm::Value* codegen(Context& ctx) = 0;
};

class StmtAST: public AST {
public:
    virtual ~StmtAST() = default;
    virtual std::string stringify(size_t indent_level = 0) = 0;
    virtual llvm::Value* codegen(Context& ctx) = 0;
};

class ExprAST: public AST {
public:
    virtual ~ExprAST() = default;
    virtual std::string stringify(size_t indent_level = 0) = 0;
    virtual llvm::Value* codegen(Context& ctx) = 0;
};

class VarDeclareAST: public StmtAST {
public:
    std::string name;
    Type& type;
    std::shared_ptr<ExprAST> expr;

    VarDeclareAST(const std::string& name, Type& type, std::shared_ptr<ExprAST> expr):
        name{name}, type{type}, expr{std::move(expr)} {}

    std::string stringify(size_t indent_level = 0);
    virtual llvm::Value* codegen(Context& ctx);
};

class FunctionAST: public StmtAST {
public:
    std::string name;
    std::vector<VarDeclareAST> parameters;
    std::vector<std::shared_ptr<StmtAST>> body;

    // FOR NOW; I just want things to work
    std::shared_ptr<ExprAST> return_type;

    FunctionAST(const std::string& name, std::vector<VarDeclareAST> parameters, std::vector<std::shared_ptr<StmtAST>> body):
        name{name}, parameters{std::move(parameters)}, body{std::move(body)} {}


    std::string stringify(size_t indent_level = 0);
    virtual llvm::Value* codegen(Context& ctx);
};

class OmgAST: public StmtAST {
public:
    std::shared_ptr<ExprAST> expr;

    OmgAST(std::shared_ptr<ExprAST> expr): expr{std::move(expr)} {}

    std::string stringify(size_t indent_level = 0);
    virtual llvm::Value* codegen(Context& ctx);
};

class ExprStmtAST: public StmtAST {
public:
    std::shared_ptr<ExprAST> expr;

    ExprStmtAST(std::shared_ptr<ExprAST> expr): expr{std::move(expr)} {}

    std::string stringify(size_t indent_level = 0);
    virtual llvm::Value* codegen(Context& ctx);
};

class BinaryExprAST: public ExprAST {
public:
    TokenType op;
    std::shared_ptr<ExprAST> lhs;
    std::shared_ptr<ExprAST> rhs;

    BinaryExprAST(TokenType op, std::shared_ptr<ExprAST> lhs, std::shared_ptr<ExprAST> rhs):
        op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {}
    
    std::string stringify(size_t indent_level);
    virtual llvm::Value* codegen(Context& ctx);
};

class CallAST: public ExprAST {
public:
    std::string callee;
    std::vector<std::shared_ptr<ExprAST>> arguments;

    CallAST(const std::string& callee, std::vector<std::shared_ptr<ExprAST>> arguments):
        callee{callee}, arguments{std::move(arguments)} {}
    
    std::string stringify(size_t indent_level);
    virtual llvm::Value* codegen(Context& ctx);
};

class PrimitiveAST: public ExprAST {
public:
    union {
        int64_t int64;
        uint64_t uint64;
        double float64;
    };
    std::string string;

    enum ValueType {INVALID, INT64, UINT64, FLOAT64, STRING} value_type;

    PrimitiveAST(): value_type{ValueType::INVALID} {}
    PrimitiveAST(int64_t int64): int64{int64}, value_type{ValueType::INT64} {}
    PrimitiveAST(uint64_t uint64): uint64{uint64}, value_type{ValueType::UINT64} {}
    PrimitiveAST(double float64): float64{float64}, value_type{ValueType::FLOAT64} {}
    PrimitiveAST(std::string string): string{std::move(string)}, value_type{ValueType::STRING} {}

    std::string stringify(size_t indent_level = 0);
    virtual llvm::Value* codegen(Context& ctx);
};

class VariableAST: public ExprAST {
public:
    std::string name;

    VariableAST(const std::string& name): name{name} {}

    std::string stringify(size_t indent_level = 0);
    virtual llvm::Value* codegen(Context& ctx);
};
