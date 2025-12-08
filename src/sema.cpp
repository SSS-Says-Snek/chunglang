#include "chung/ast.hpp"
#include "chung/token.hpp"
#include "chung/type.hpp"
#include "chung/sema.hpp"
#include <llvm/Support/ErrorHandling.h>
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
        std::cout << "TODOREPLACE but redeclared variable\n";
        return false;
    }

    scopes.back().emplace_back(&decl);
    return true;
}

std::unique_ptr<ResolvedOmg> Sema::resolve_omg(const OmgAST& /*omg*/) {
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

    // Every stmt should be covered already; if not, implementation error
    llvm_unreachable("Unhandled statement in Sema::resolve_stmt");
}

std::unique_ptr<ResolvedExpr> Sema::resolve_expr(const ExprAST& expr) {
    if (const auto* primitive = dynamic_cast<const PrimitiveAST*>(&expr)) {
        return resolve_primitive(*primitive);
    }

    if (const auto* if_expr = dynamic_cast<const IfExprAST*>(&expr)) {
        return resolve_if_expr(*if_expr);
    }

    if (const auto* binary_expr = dynamic_cast<const BinaryExprAST*>(&expr)) {
        return resolve_binary_expr(*binary_expr);
    }

    if (const auto* variable = dynamic_cast<const VariableAST*>(&expr)) {
        return resolve_variable(*variable);
    }

    if (const auto* call = dynamic_cast<const CallAST*>(&expr)) {
        return resolve_call(*call);
    }

    // Every expr should be covered already; if not, implementation error
    llvm_unreachable("Unhandled expression in Sema::resolve_expr");
}

std::unique_ptr<ResolvedIfExpr> Sema::resolve_if_expr(const IfExprAST& if_expr) {
}

std::unique_ptr<ResolvedBinaryExpr> Sema::resolve_binary_expr(const BinaryExprAST& binary_expr) {
    std::unique_ptr<ResolvedExpr> resolved_lhs = resolve_expr(*binary_expr.lhs);
    std::unique_ptr<ResolvedExpr> resolved_rhs = resolve_expr(*binary_expr.rhs);
    if (!resolved_lhs || !resolved_rhs) {
        return nullptr;
    }

    if (resolved_lhs->type.ty != resolved_rhs->type.ty) { // TODO: operator up/down, struct, operator overloading?
        std::cout << "TODOREPLACE but binary expression contains two different types\n";
        return nullptr;
    }

    return std::make_unique<ResolvedBinaryExpr>(binary_expr.loc, binary_expr.op, std::move(resolved_lhs),
                                                std::move(resolved_rhs));
}

std::unique_ptr<ResolvedPrimitive> Sema::resolve_primitive(const PrimitiveAST& primitive) {
    switch (primitive.type) {
        case TokenType::INT64: {
            int64_t int64 = std::stoll(primitive.value);
            return std::make_unique<ResolvedPrimitive>(primitive.loc, int64);
        }
        case TokenType::UINT64: {
            uint64_t uint64 = std::stoull(primitive.value);
            return std::make_unique<ResolvedPrimitive>(primitive.loc, uint64);
        }
        case TokenType::FLOAT64: {
            double float64 = std::stod(primitive.value);
            return std::make_unique<ResolvedPrimitive>(primitive.loc, float64);
        }
        case TokenType::STRING: {
            return std::make_unique<ResolvedPrimitive>(primitive.loc, primitive.value);
        }
        default:
            llvm_unreachable("Invalid token");
    }
}

std::unique_ptr<ResolvedVariable> Sema::resolve_variable(const VariableAST& variable) {
    auto [resolved_decl, scope_level] = lookup_declaration(variable.name);
    if (!resolved_decl) {
        std::cout << "TODOREPLACE but variable/symbol " + variable.name + " not found\n";
        return nullptr;
    }

    auto* resolved_var_decl = dynamic_cast<ResolvedDecl*>(resolved_decl);
    if (!resolved_var_decl) {
        std::cout << "TODOREPLACE but symbol " + variable.name + " is not a variable\n";
    }

    return std::make_unique<ResolvedVariable>(variable.loc, resolved_var_decl);
}

std::unique_ptr<ResolvedFunction> Sema::resolve_function(const FunctionAST& function) {
    std::optional<Type> return_type = resolve_type(function.type);

    if (!return_type) {
        std::cout << "TODOREPLACE but invalid type\n";
        return nullptr;
    }

    if (function.name == "main") {
        if (return_type->ty != Ty::VOID) {
            std::cout << "TODOREPLACE but main has to return void\n";
            return nullptr;
        }

        if (!function.parameters.empty()) {
            std::cout << "TODOREPLACE but main should have 0 parameters\n";
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

    return std::make_unique<ResolvedFunction>(function.loc, function.name, std::move(resolved_params), *return_type,
                                              nullptr);
}

std::unique_ptr<ResolvedParamDeclare> Sema::resolve_param_decl(const ParamDeclareAST& param) {
    std::optional<Type> type = resolve_type(param.type);

    if (!type || type->ty == Ty::VOID) {
        std::cout << "TODOREPLACE but invalid type for parameter\n";
        return nullptr;
    }

    return std::make_unique<ResolvedParamDeclare>(param.loc, param.name, *type);
}

std::unique_ptr<ResolvedCall> Sema::resolve_call(const CallAST& call) {
    const auto& [resolved_decl, scope_level] = lookup_declaration(call.callee);
    if (!resolved_decl) {
        std::cout << "TODOREPLACE but can't find callee " + call.callee + '\n';
        return nullptr;
    }

    const auto* resolved_function = dynamic_cast<ResolvedFunction*>(resolved_decl);
    if (!resolved_function) {
        std::cout << "TODOREPLACE but callee " + call.callee + " is not a function\n";
        return nullptr;
    }

    std::vector<std::unique_ptr<ResolvedExpr>> resolved_arguments;
    for (size_t i = 0; i < call.arguments.size(); i++) {
        const auto& argument = call.arguments[i];

        std::unique_ptr<ResolvedExpr> resolved_expr = resolve_expr(*argument);
        if (!resolved_expr) {
            return nullptr;
        }
        if (resolved_expr->type.ty != resolved_function->parameters[i]->type.ty) {
            std::cout << "TODOREPLACE but argument type is mismatching with parameter type\n";
            return nullptr;
        }

        resolved_arguments.emplace_back(std::move(resolved_expr));
    }

    return std::make_unique<ResolvedCall>(call.loc, *resolved_function, std::move(resolved_arguments));
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

std::unique_ptr<ResolvedFunction> generate_print() {
    auto print_loc = SourceLocation{0, 0, 0};
    std::vector<std::unique_ptr<ResolvedParamDeclare>> params;
    auto n = std::make_unique<ResolvedParamDeclare>(print_loc, "n", Type::int64);
    params.push_back(std::move(n));

    auto block = std::make_unique<ResolvedBlock>(print_loc, std::vector<std::unique_ptr<ResolvedStmt>>());
    return std::make_unique<ResolvedFunction>(print_loc, "print", std::move(params), Type::void_, std::move(block));
}

std::vector<std::unique_ptr<ResolvedStmt>> Sema::resolve() {
    std::vector<std::unique_ptr<ResolvedStmt>> resolved_ast;

    // Will emplace and pop as needed
    ScopeRAII global_scope{this};

    std::vector<const FunctionAST*> unresolved_functions;

    // Add print decl rq manually
    auto* e = resolved_ast.emplace_back(generate_print()).get();
    add_declaration(*dynamic_cast<ResolvedDecl*>(e));

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

    for (size_t i = 1; i < resolved_ast.size(); i++) {
        std::unique_ptr<ResolvedStmt>& stmt = resolved_ast[i];
        if (auto* function = dynamic_cast<ResolvedFunction*>(stmt.get())) {
            current_function = function;
            // Statement is a function declaration, so new scope
            ScopeRAII parameter_scope{this};

            for (auto&& param : function->parameters) {
                add_declaration(*param);
            }

            auto resolved_body = resolve_block(
                *dynamic_cast<FunctionAST*>(ast[i - 1].get())->body); // This is the worst thing I've ever written
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

    return resolved_ast;
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
