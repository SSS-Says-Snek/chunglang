#include "chung/context.hpp"
#include "chung/token.hpp"
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <utility>

class ResolvedAST {
public:
    virtual ~ResolvedAST() = default;
    // virtual std::string stringify(size_t indent_level = 0) = 0;
    // virtual llvm::Value* codegen(Context& ctx) = 0;
};

class ResolvedStmt : public ResolvedAST {
public:
    SourceLocation loc;

    ResolvedStmt(SourceLocation loc) : loc{loc} {
    }

    // std::string stringify(size_t indent_level = 0) override = 0;
    // llvm::Value* codegen(Context& ctx) override = 0;
};

class ResolvedExpr : public ResolvedAST {
public:
    SourceLocation loc;
    // DIFFERENT FROM ExprAST: Type
    Type type{Type::invalid};

    ResolvedExpr(SourceLocation loc, Type type) : loc{loc}, type{std::move(type)} {
    }

    // std::string stringify(size_t indent_level = 0) override = 0;
    // llvm::Value* codegen(Context& ctx) override = 0;
};

class ResolvedBlock : public ResolvedStmt {
public:
    std::vector<std::unique_ptr<ResolvedStmt>> body;

    ResolvedBlock(SourceLocation loc, std::vector<std::unique_ptr<ResolvedStmt>> body)
        : ResolvedStmt(loc), body{std::move(body)} {
    }

    // std::string stringify(size_t indent_level = 0) override;
};

class ResolvedDecl : public ResolvedStmt {
public:
    std::string name;

    ResolvedDecl(SourceLocation loc, std::string name) : ResolvedStmt(loc), name{std::move(name)} {
    }

    // std::string stringify(size_t indent_level = 0) override = 0;
    // llvm::Value* codegen(Context& ctx) override = 0;
};

class ResolvedVarDeclare : public ResolvedDecl {
public:
    Type type;
    std::unique_ptr<ResolvedExpr> expr;

    ResolvedVarDeclare(SourceLocation loc, std::string name, Type type, std::unique_ptr<ResolvedExpr> expr)
        : ResolvedDecl(loc, std::move(name)), type{std::move(type)}, expr{std::move(expr)} {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedParamDeclare : public ResolvedDecl {
public:
    Type type;

    ResolvedParamDeclare(SourceLocation loc, std::string name, Type type)
        : ResolvedDecl(loc, std::move(name)), type{std::move(type)} {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedFunction : public ResolvedDecl {
public:
    std::vector<std::unique_ptr<ResolvedParamDeclare>> parameters;
    Type return_type;
    std::unique_ptr<ResolvedBlock> body;


    ResolvedFunction(SourceLocation loc, std::string name, std::vector<std::unique_ptr<ResolvedParamDeclare>> parameters, Type return_type,
                     std::unique_ptr<ResolvedBlock> body)
        : ResolvedDecl(loc, std::move(name)), parameters{std::move(parameters)}, return_type{std::move(return_type)}, body{std::move(body)} {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedOmg : public ResolvedStmt {
public:
    std::unique_ptr<ResolvedExpr> expr;

    ResolvedOmg(SourceLocation loc, std::unique_ptr<ResolvedExpr> expr) : ResolvedStmt(loc), expr{std::move(expr)} {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedExprStmt : public ResolvedStmt {
public:
    std::unique_ptr<ResolvedExpr> expr;

    ResolvedExprStmt(SourceLocation loc, std::unique_ptr<ResolvedExpr> expr)
        : ResolvedStmt(loc), expr{std::move(expr)} {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedBinaryExpr : public ResolvedExpr {
public:
    TokenType op;
    std::unique_ptr<ResolvedExpr> lhs;
    std::unique_ptr<ResolvedExpr> rhs;

    ResolvedBinaryExpr(SourceLocation loc, TokenType op, std::unique_ptr<ResolvedExpr> lhs,
                       std::unique_ptr<ResolvedExpr> rhs)
        : ResolvedExpr(loc, lhs->type), op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {
    }

    // std::string stringify(size_t indent_level) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedCall : public ResolvedExpr {
public:
    std::unique_ptr<ResolvedFunction> callee;
    std::vector<std::unique_ptr<ResolvedExpr>> arguments;

    ResolvedCall(SourceLocation loc, std::unique_ptr<ResolvedFunction> callee,
                 std::vector<std::unique_ptr<ResolvedExpr>> arguments)
        : ResolvedExpr(loc, callee->return_type), callee{std::move(callee)}, arguments{std::move(arguments)} {
    }

    // std::string stringify(size_t indent_level) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedIfExpr : public ResolvedExpr {
public:
    std::unique_ptr<ResolvedExpr> condition;
    std::unique_ptr<ResolvedBlock> body;
    std::unique_ptr<ResolvedBlock> else_body;

    ResolvedIfExpr(SourceLocation loc, std::unique_ptr<ResolvedExpr> condition,
                   std::unique_ptr<ResolvedBlock> body,
                   std::unique_ptr<ResolvedBlock> else_body)
        : ResolvedExpr(loc, Type::none), condition{std::move(condition)}, body{std::move(body)},
          else_body{std::move(else_body)} { // Too lazy for type; TODO
    }

    // std::string stringify(size_t indent_level) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedPrimitive : public ResolvedExpr {
public:
    union {
        int64_t int64{};
        uint64_t uint64;
        double float64;
    };
    std::string string;

    enum ValueType : int8_t { INVALID, INT64, UINT64, FLOAT64, STRING } value_type;

    ResolvedPrimitive(SourceLocation loc) : ResolvedExpr(loc, Type::invalid), value_type{ValueType::INVALID} {
    }
    ResolvedPrimitive(SourceLocation loc, int64_t int64)
        : ResolvedExpr(loc, Type::int64), int64{int64}, value_type{ValueType::INT64} {
    }
    ResolvedPrimitive(SourceLocation loc, uint64_t uint64)
        : ResolvedExpr(loc, Type::uint64), uint64{uint64}, value_type{ValueType::UINT64} {
    }
    ResolvedPrimitive(SourceLocation loc, double float64)
        : ResolvedExpr(loc, Type::float64), float64{float64}, value_type{ValueType::FLOAT64} {
    }
    ResolvedPrimitive(SourceLocation loc, std::string string)
        : ResolvedExpr(loc, Type::string), string{std::move(string)}, value_type{ValueType::STRING} {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};

class ResolvedVariable : public ResolvedExpr {
public:
    std::unique_ptr<ResolvedVarDeclare> declaration;

    ResolvedVariable(SourceLocation loc, std::unique_ptr<ResolvedVarDeclare> declaration)
        : ResolvedExpr(loc, declaration->type) {
    }

    // std::string stringify(size_t indent_level = 0) override;
    // llvm::Value* codegen(Context& ctx) override;
};
