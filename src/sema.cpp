#include "chung/ast.hpp"
#include "chung/type.hpp"
#include "chung/sema.hpp"
#include <algorithm>
#include <memory>
#include <iostream>

std::pair<ResolvedDecl*, int> Sema::lookup_declaration(const std::string& name) {
    // 0 is innermost, more positive = more outer
    int scope_level = 0;
    
    for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
        for (auto&& declaration : *it) {
            if (declaration->name != name) {
                continue;
            }
            return {declaration, scope_level};
        }

        scope_level++;
    }

    return {nullptr, -1}; // Can't find ts in tables
}

bool Sema::add_declaration(ResolvedDecl& decl) {
    const auto& [found_decl, scope_level] = lookup_declaration(decl.name);
    
    if (found_decl && scope_level == 0) { // If already defined within current scope
        std::cout << "TODOREPLACE but redeclared variable";
        return false;
    }

    scopes.back().emplace_back(&decl);
    return true;
}

std::unique_ptr<ResolvedOmg> Sema::resolve_omg(const OmgAST&  /*omg*/) {
    // What does omgast even do i forgor
    return nullptr;
}

std::unique_ptr<ResolvedStmt> Sema::resolve_stmt(const StmtAST& stmt) {
    if (const auto* func = dynamic_cast<const FunctionAST*>(&stmt)) {
        return resolve_function(*func);
    }

    if (const auto* omg = dynamic_cast<const OmgAST*>(&stmt)) {
        return resolve_omg(*omg);
    }

    if (const auto* expr = dynamic_cast<const ExprStmtAST*>(&stmt)) {
        auto resolved_expr = resolve_expr(*expr->expr);
        return std::make_unique<ResolvedExprStmt>(stmt.loc, std::move(resolved_expr));
    }
}

std::unique_ptr<ResolvedExpr> Sema::resolve_expr(const ExprAST& expr) {

}

std::unique_ptr<ResolvedFunction> Sema::resolve_function(const FunctionAST& function) {
    std::optional<Type> return_type = resolve_type(function.return_type);

    if (!return_type) {
        std::cout << "TODOREPLACE but invalid type";
        return nullptr;
    }

    if (function.name == "main") {
        if (return_type->ty != Ty::VOID) {
            std::cout << "TODOREPLACE but main has to return void";
            return nullptr;
        }

        if (!function.parameters.empty()) {
            std::cout << "TODOREPLACE but main should have 0 parameters";
            return nullptr;
        }
    }

    ScopeRAII param_scope{this};
    std::vector<std::unique_ptr<ResolvedParamDeclare>> resolved_params;

    for (auto&& param : function.parameters) {
        std::unique_ptr<ResolvedParamDeclare> resolved_param = resolve_param_decl(param);
        
        if (!resolved_param || !add_declaration(*resolved_param)) {
            return nullptr;
        }
        resolved_params.push_back(std::move(resolved_param));
    }

    return std::make_unique<ResolvedFunction>(function.loc, function.name, std::move(resolved_params), *return_type, nullptr);
}

std::unique_ptr<ResolvedParamDeclare> Sema::resolve_param_decl(const ParamDeclareAST& param) {
    std::optional<Type> type = resolve_type(param.type);

    if (!type || type->ty == Ty::VOID) {
        std::cout << "TODOREPLACE but invalid type for parameter";
        return nullptr;
    }
    
    return std::make_unique<ResolvedParamDeclare>(param.loc, param.name, *type);
}

std::unique_ptr<ResolvedCall> Sema::resolve_call(const CallAST& call) {

}

std::unique_ptr<ResolvedBlock> Sema::resolve_block(const BlockAST& block) {
    std::vector<std::unique_ptr<ResolvedStmt>> resolved_statements;
    bool error = false;

    ScopeRAII block_scope{this};
    for (auto&& stmt : block.body) {
        auto resolved_statement = resolve_stmt(*stmt);
        error |= !resolved_statements.emplace_back(std::move(resolved_statement));
        if (error) {
            continue;
        }
    }

    if (error) {
        return nullptr;
    }

    return std::make_unique<ResolvedBlock>(block.loc, std::move(resolved_statements));
}

std::optional<Type> Sema::resolve_type(Type parsed_type) {
    if (parsed_type.ty == Ty::USER) {
        return std::nullopt;
    }
    return parsed_type;
}

std::vector<std::unique_ptr<ResolvedStmt>> Sema::resolve() {
    std::vector<std::unique_ptr<ResolvedStmt>> resolved_ast;

    // Will emplace and pop as needed
    ScopeRAII global_scope{this};

    std::vector<const FunctionAST*> unresolved_functions;

    // First pass: just add the symbols of the global declarations (for stuff like forward referencing)
    bool error = false;
    for (auto&& stmt : ast) {
        std::cout << stmt->loc.line << '\n';
        if (const auto* function = dynamic_cast<const FunctionAST*>(stmt.get())) {
            auto resolved_decl = resolve_function(*function);

            if (!resolved_decl || !add_declaration(*resolved_decl)) {
                // Error propagated
                error = true;
                continue;
            }

            resolved_ast.emplace_back(std::move(resolved_decl));
        }

        // auto resolved_stmt = dynamic_cast<ResolvedDecl&>(*resolve_stmt(*stmt));

        // TODO: replace this dynamic cast by converting all vectors of ResolvedStmt / StmtAST to ResolvedDecl / DeclAST

    }

    if (error) {
        return {};
    }

    // Second pass

    for (int i = 0; i < resolved_ast.size(); i++) {
        std::unique_ptr<ResolvedStmt>& stmt = resolved_ast[i];
        if (auto* function = dynamic_cast<ResolvedFunction*>(stmt.get())) {
            current_function = function;
            // Statement is a function declaration, so new scope
            ScopeRAII parameter_scope{this};

            for (auto&& param : function->parameters) {
                add_declaration(*param);
            }

            auto resolved_body = resolve_block(*dynamic_cast<FunctionAST*>(ast[i].get())->body); // This is the worst thing I've ever written
            if (!resolved_body) {
                error = true;
                continue;
            }

            current_function->body = std::move(resolved_body);
        }
    }

    if (error) {
        return {};
    }

    return {};
}

// std::unique_ptr<ResolvedStmt> StmtAST::resolve() {
//     std::cout << "StmtAST::resolve\n";
//     return nullptr;
// }
//
// std::unique_ptr<ResolvedFunction> FunctionAST::resolve() {
//     std::cout << "FunctionAST::resolve\n";
//     return nullptr;
// }
//
// std::unique_ptr<ResolvedPrimitive> PrimitiveAST::resolve() {
    // switch (token.type) {
    //     case TokenType::INT64: {
    //         int64_t int64 = std::stoll(token.text);
    //         return std::make_unique<PrimitiveAST>(int64);
    //     }
    //     case TokenType::UINT64: {
    //         uint64_t uint64 = std::stoull(token.text);
    //         return std::make_unique<PrimitiveAST>(uint64);
    //     }
    //     case TokenType::FLOAT64: {
    //         double float64 = std::stod(token.text);
    //         return std::make_unique<PrimitiveAST>(float64);
    //     }
    //     case TokenType::STRING: {
    //         return std::make_unique<PrimitiveAST>(token.text);
    //     }
    //     default:
    //         // Invalid token
    //         throw push_exception("Invalid token in expression", token);
    // }

// }
