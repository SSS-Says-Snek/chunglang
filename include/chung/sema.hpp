#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "ast.hpp"
#include "chung/error.hpp"
#include "chung/token.hpp"
#include "resolved_ast.hpp"

class SemaException : public Exception {
public:
    std::string exception_message;
    SourceLocation loc;

    const std::string& source_line;

    SemaException(std::string exception_message, SourceLocation loc, const std::string& source_line);
    std::string write(const std::vector<std::string>& source_lines) override;
};

class Sema {
private:
    std::vector<SemaException> exceptions;
public:
    std::vector<std::unique_ptr<StmtAST>> ast;
    const std::vector<std::string>& source_lines;

    // 1 scope = std::vector<ResolvedDecl*>, multiple will be a chain
    std::vector<std::vector<ResolvedDecl*>> scopes;

    ResolvedFunction* current_function{nullptr};

    explicit Sema(std::vector<std::unique_ptr<StmtAST>> ast, const std::vector<std::string>& source_lines) : ast{std::move(ast)}, source_lines{source_lines} {
    }

    std::vector<std::unique_ptr<ResolvedStmt>> resolve();

    std::unique_ptr<ResolvedStmt> resolve_stmt(const StmtAST& stmt);
    std::unique_ptr<ResolvedCall> resolve_call(const CallAST& call);
    std::unique_ptr<ResolvedBinaryExpr> resolve_binop(const BinaryExprAST& binop);
    std::unique_ptr<ResolvedFunction> resolve_function(const FunctionAST& function);
    std::unique_ptr<ResolvedParamDeclare> resolve_param_decl(const ParamDeclareAST& param);
    std::unique_ptr<ResolvedBlock> resolve_block(const BlockAST& block);
    std::unique_ptr<ResolvedExpr> resolve_expr(const ExprAST& expr);
    std::unique_ptr<ResolvedExpr> resolve_expr_stmt(const ExprStmtAST& expr_stmt);
    static std::unique_ptr<ResolvedOmg> resolve_omg(const OmgAST& block);
    std::unique_ptr<ResolvedIfExpr> resolve_if_expr(const IfExprAST& if_expr);
    std::unique_ptr<ResolvedBinaryExpr> resolve_binary_expr(const BinaryExprAST& binary_expr);
    static std::unique_ptr<ResolvedPrimitive> resolve_primitive(const PrimitiveAST& primitive);
    std::unique_ptr<ResolvedVariable> resolve_variable(const VariableAST& variable);
    static std::optional<Type> resolve_type(Type parsed_type);

    std::pair<ResolvedDecl*, int> lookup_declaration(const std::string& name);
    bool add_declaration(ResolvedDecl& decl);

    void add_scope() {
        scopes.emplace_back();
    }

    void pop_scope() {
        scopes.pop_back();
    }

    SemaException push_exception(const std::string& exception_message, const SourceLocation& loc) {
        SemaException exception{exception_message, loc, source_lines[loc.line - 1]};
        exceptions.push_back(exception);
        return exception;
    }

    const std::vector<SemaException>& get_exceptions() {
        return exceptions;
    }
};

class ScopeRAII {
    Sema* sema;

public:
    explicit ScopeRAII(Sema* sema) : sema{sema} {
        sema->add_scope();
    }

    ScopeRAII(const ScopeRAII&) = default;
    ScopeRAII(ScopeRAII&&) = default;
    ScopeRAII& operator=(const ScopeRAII&) = default;
    ScopeRAII& operator=(ScopeRAII&&) = default;

    ~ScopeRAII() {
        sema->pop_scope();
    }
};
