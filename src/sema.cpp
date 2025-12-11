#include "chung/resolved_ast.hpp"
#include "chung/ast.hpp"
#include "chung/token.hpp"
#include "chung/type.hpp"
#include "chung/utils/ansi.hpp"
#include "chung/sema.hpp"
#include <llvm/Support/ErrorHandling.h>
#include <iostream>
#include <memory>

#define HANDLE_MAKE_VAR(identifier, initialization)                                                                    \
    auto identifier = (initialization);                                                                                \
    if (!identifier) return nullptr;

SemaException::SemaException(std::string exception_message, SourceLocation loc, const std::string& source_line)
    : exception_message{std::move(exception_message)}, loc{loc}, source_line{source_line} {
}

std::string SemaException::write(const std::vector<std::string>& source_lines) {
    std::string string{ANSI_RED};
    string += "SemaException at line " + std::to_string(loc.line) + " column " +
              std::to_string(loc.column) + ":\n" + ANSI_RESET;
    std::string carets;

    for (size_t i = 0; i <= source_line.length(); i++) {
        if (i == loc.column) {
            carets += ANSI_RED;
        }
        if (i == loc.column + loc.token_length) {
            carets += ANSI_RESET;
        }

        if (loc.column <= i && i < loc.column + loc.token_length) {
            carets += '^';
        } else {
            carets += '~';
        }
    }

    if (loc.line > 0) {
        string += "|\t" + source_lines[loc.line - 1 - 1] + '\n';
    }

    string += "|\t";
    string += std::string{ANSI_RED} + source_line + ANSI_RESET + '\n';
    string += "|\t" + carets + '\n';

    if (loc.line < source_lines.size()) {
        string += "|\t" + source_lines[loc.line] + '\n';
    }
    string += ANSI_RED + exception_message + ANSI_RESET + '\n';

    return string;
}

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
    HANDLE_MAKE_VAR(condition, resolve_expr(*if_expr.condition))
    // TODO: Check if condition expr is actually comparable and evaluates

    HANDLE_MAKE_VAR(resolved_body, resolve_block(*if_expr.body))

    std::unique_ptr<ResolvedBlock> resolved_else_body;
    if (if_expr.else_body) {
        resolved_else_body = resolve_block(*if_expr.else_body);
        if (!resolved_else_body) {
            return nullptr;
        }

        // TODO: Better type checking here
        if (resolved_body->type != resolved_else_body->type) {
            SourceLocation resolved_body_loc;
            if (resolved_body->return_value) {
                resolved_body_loc = resolved_body->return_value->loc;
            } else {
                resolved_body_loc = resolved_body->loc;
            }
            push_exception("Mismatched types between `if` and `else` bodies of if-expression", resolved_body_loc);

            if (resolved_else_body && resolved_else_body->return_value) {
                resolved_body_loc = resolved_else_body->return_value->loc;
            } else {
                resolved_body_loc = resolved_else_body->loc;
            }
            push_exception("Mismatched types between `if` and `else` bodies of if-expression", resolved_body_loc);
            return nullptr;
        }
    }

    return std::make_unique<ResolvedIfExpr>(if_expr.loc, std::move(resolved_body->type), std::move(condition), std::move(resolved_body),
                                            std::move(resolved_else_body));
}

std::unique_ptr<ResolvedBinaryExpr> Sema::resolve_binary_expr(const BinaryExprAST& binary_expr) {
    HANDLE_MAKE_VAR(resolved_lhs, resolve_expr(*binary_expr.lhs))
    HANDLE_MAKE_VAR(resolved_rhs, resolve_expr(*binary_expr.rhs))

    if (resolved_lhs->type.ty != resolved_rhs->type.ty) { // TODO: operator up/down, struct, operator overloading?
        push_exception("Binary expression contains two mismatching types", binary_expr.loc);
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
        push_exception("Variable '" + variable.name + "' not found", variable.loc);
        return nullptr;
    }

    auto* resolved_var_decl = dynamic_cast<ResolvedDecl*>(resolved_decl);
    if (!resolved_var_decl) {
        push_exception("Symbol '" + variable.name + "' is not a variable", variable.loc);
        return nullptr;
    }

    return std::make_unique<ResolvedVariable>(variable.loc, resolved_var_decl);
}

std::unique_ptr<ResolvedFunction> Sema::resolve_function(const FunctionAST& function) {
    std::optional<Type> return_type = resolve_type(function.type);

    if (!return_type) {
        push_exception("Invalid return type '" + function.type.name + "' for function '" + function.name + "'", function.loc);
        return nullptr;
    }

    if (function.name == "main") {
        if (return_type->ty != Ty::VOID) {
            push_exception("Function 'main' must return void", function.loc);
            return nullptr;
        }

        if (!function.parameters.empty()) {
            push_exception("Function 'main' must contain zero parameters", function.loc);
            return nullptr;
        }
    }

    ScopeRAII param_scope{this};
    std::vector<std::unique_ptr<ResolvedParamDeclare>> resolved_params;

    for (auto&& param : function.parameters) {
        HANDLE_MAKE_VAR(resolved_param, resolve_param_decl(param))

        if (!add_declaration(*resolved_param)) {
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
        push_exception("Invalid type for parameter '" + param.name + "'", param.loc);
        return nullptr;
    }

    return std::make_unique<ResolvedParamDeclare>(param.loc, param.name, *type);
}

std::unique_ptr<ResolvedCall> Sema::resolve_call(const CallAST& call) {
    const auto& [resolved_decl, scope_level] = lookup_declaration(call.callee);
    if (!resolved_decl) {
        push_exception("Cannot find function '" + call.callee + "'", call.loc);
        return nullptr;
    }

    const auto* resolved_function = dynamic_cast<ResolvedFunction*>(resolved_decl);
    if (!resolved_function) {
        push_exception("Callee '" + call.callee + "' is not a function", call.loc);
        return nullptr;
    }

    std::vector<std::unique_ptr<ResolvedExpr>> resolved_arguments;
    for (size_t i = 0; i < call.arguments.size(); i++) {
        const auto& argument = call.arguments[i];

        HANDLE_MAKE_VAR(resolved_expr, resolve_expr(*argument))
        if (resolved_expr->type.ty != resolved_function->parameters[i]->type.ty) {
            push_exception("Argument and parameter types do not match; expected " + resolved_function->parameters[i]->type.name + ", found " + resolved_expr->type.name, call.loc);
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

    std::unique_ptr<ResolvedExpr> resolved_return_type;
    if (block.return_value) {
        resolved_return_type = resolve_expr(*block.return_value);
    }

    return std::make_unique<ResolvedBlock>(block.loc, std::move(resolved_statements), std::move(resolved_return_type));
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

    auto block = std::make_unique<ResolvedBlock>(print_loc, std::vector<std::unique_ptr<ResolvedStmt>>(), nullptr);
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

            auto resolved_body = resolve_block( // ast[i - 1] because the first resolved_ast is `print`
                *dynamic_cast<FunctionAST*>(ast[i - 1].get())->body); // This is the worst thing I've ever written
            if (!resolved_body) {
                error = true;
                continue;
            }

            if (resolved_body->return_value && resolved_body->return_value->type != function->type) {
                push_exception("Function '" + function->name + "' body's type of " + resolved_body->return_value->type.name + " does not match return type of " + function->type.name, resolved_body->loc);
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
