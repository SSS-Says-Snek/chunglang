#include "chung/context.hpp"
#include "chung/token.hpp"
#include <llvm/IR/Value.h>
#include <string>

class ResolvedAST {
public:
    virtual ~ResolvedAST() = default;
    virtual std::string stringify(size_t indent_level = 0) = 0;
    virtual llvm::Value* codegen(Context& ctx) = 0;
};

class ResolvedStmt : public ResolvedAST {
public:
    std::string stringify(size_t indent_level = 0) override = 0;
    llvm::Value* codegen(Context& ctx) override = 0;
};

class ResolvedExpr : public ResolvedAST {
public:
    // DIFFERENT FROM ExprAST: Type
    Type type{Type::tinvalid};

    std::string stringify(size_t indent_level = 0) override = 0;
    llvm::Value* codegen(Context& ctx) override = 0;
};

class ResolvedVarDeclare : public ResolvedStmt {
public:
    std::string name;
    Type& type;
    std::unique_ptr<ResolvedExpr> expr;

    ResolvedVarDeclare(std::string name, Type& type, std::unique_ptr<ResolvedExpr> expr)
        : name{std::move(name)}, type{type}, expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedFunction : public ResolvedStmt {
public:
    std::string name;
    std::vector<ResolvedVarDeclare> parameters;
    std::vector<std::unique_ptr<ResolvedStmt>> body;

    // FOR NOW; I just want things to work
    Type return_type{Type::tinvalid};

    ResolvedFunction(std::string name, std::vector<ResolvedVarDeclare> parameters, std::vector<std::unique_ptr<ResolvedStmt>> body)
        : name{std::move(name)}, parameters{std::move(parameters)}, body{std::move(body)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedOmg : public ResolvedStmt {
public:
    std::unique_ptr<ResolvedExpr> expr;

    ResolvedOmg(std::unique_ptr<ResolvedExpr> expr) : expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedExprStmt : public ResolvedStmt {
public:
    std::unique_ptr<ResolvedExpr> expr;

    ResolvedExprStmt(std::unique_ptr<ResolvedExpr> expr) : expr{std::move(expr)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedBinaryExpr : public ResolvedExpr {
public:
    TokenType op;
    std::unique_ptr<ResolvedExpr> lhs;
    std::unique_ptr<ResolvedExpr> rhs;

    ResolvedBinaryExpr(TokenType op, std::unique_ptr<ResolvedExpr> lhs, std::unique_ptr<ResolvedExpr> rhs)
        : op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {
    }

    std::string stringify(size_t indent_level) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedCall : public ResolvedExpr {
public:
    std::string callee;
    std::vector<std::unique_ptr<ResolvedExpr>> arguments;

    ResolvedCall(std::string callee, std::vector<std::unique_ptr<ResolvedExpr>> arguments)
        : callee{std::move(callee)}, arguments{std::move(arguments)} {
    }

    std::string stringify(size_t indent_level) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedIfExpr : public ResolvedExpr {
public:
    std::unique_ptr<ResolvedExpr> condition;
    std::vector<std::unique_ptr<ResolvedStmt>> body;
    std::vector<std::unique_ptr<ResolvedStmt>> else_body;

    ResolvedIfExpr(std::unique_ptr<ResolvedExpr> condition, std::vector<std::unique_ptr<ResolvedStmt>> body,
              std::vector<std::unique_ptr<ResolvedStmt>> else_body)
        : condition{std::move(condition)}, body{std::move(body)}, else_body{std::move(else_body)} {
    }

    std::string stringify(size_t indent_level) override;
    llvm::Value* codegen(Context& ctx) override;
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

    ResolvedPrimitive() : value_type{ValueType::INVALID} {
    }
    ResolvedPrimitive(int64_t int64) : int64{int64}, value_type{ValueType::INT64} {
    }
    ResolvedPrimitive(uint64_t uint64) : uint64{uint64}, value_type{ValueType::UINT64} {
    }
    ResolvedPrimitive(double float64) : float64{float64}, value_type{ValueType::FLOAT64} {
    }
    ResolvedPrimitive(std::string string) : string{std::move(string)}, value_type{ValueType::STRING} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
};

class ResolvedVariable : public ResolvedExpr {
public:
    std::string name;

    ResolvedVariable(std::string name) : name{std::move(name)} {
    }

    std::string stringify(size_t indent_level = 0) override;
    llvm::Value* codegen(Context& ctx) override;
};
