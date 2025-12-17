#include "chung/resolved_ast.hpp"
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>

llvm::Value* ResolvedVarDeclare::codegen(Context& ctx) {
    llvm::Function* current_function = ctx.builder.GetInsertBlock()->getParent();

    llvm::AllocaInst* var = ctx.allocate_stack_variable(name, ctx.llvm_types.at(type));

    if (expr) {
        ctx.builder.CreateStore(expr->codegen(ctx), var);
    }

    ctx.named_values[this] = var;
    return nullptr;
}

llvm::Value* ResolvedParamDeclare::codegen(Context& /*ctx*/) {
    // Not using for now I think
    std::cout << "TODOREPLACE but you shouldn't be here.\n";
    return nullptr;
}

llvm::Value* ResolvedBlock::codegen(Context& ctx, bool create_ret_instructions) {
    for (auto& stmt : body) {
        stmt->codegen(ctx);
    }

    // TODO: FLAWED: MUST MOVE THIS OUTSIDE SO THAT BLOCKS WITH RETURN VALUES CAN DO SOMETHING OTHER THAN RETURN
    llvm::Value* return_expr = nullptr;
    if (return_value) {
        return_expr = return_value->codegen(ctx);
        if (create_ret_instructions) {
            ctx.builder.CreateRet(return_expr);
        }
    }
    return return_expr;
}

llvm::Value* ResolvedFunction::codegen(Context& ctx) {
    if (name == "print") { // Nah no need
        return nullptr;
    }

    std::vector<llvm::Type*> parameter_types;
    parameter_types.reserve(parameters.size());
    for (auto& parameter : parameters) {
        parameter_types.push_back(ctx.llvm_types.at(parameter->type));
    }

    llvm::FunctionType* function_type = llvm::FunctionType::get(ctx.llvm_types.at(type), parameter_types, false);
    llvm::Function* function =
        llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name, ctx.module.get());


    llvm::BasicBlock* function_block = llvm::BasicBlock::Create(ctx.context, "entry", function);
    ctx.builder.SetInsertPoint(function_block);

    // Variable insert point so that declarations can get put to the function entry block
    llvm::Value* undef = llvm::UndefValue::get(ctx.builder.getInt32Ty());
    ctx.variable_insert_point = new llvm::BitCastInst(undef, undef->getType(), "alloca.placeholder", function_block);

    ctx.named_values.clear();

    // Set parameter names
    size_t i = 0;
    for (auto& function_parameter : function->args()) {
        auto& parameter_name = parameters[i]->name;
        function_parameter.setName(parameter_name);
        ctx.named_values[parameters[i].get()] = &function_parameter;

        i++;
    }
    body->codegen(ctx, true);

    // Void FOR NOW
    if (type.ty == Ty::VOID) {
        ctx.builder.CreateRet(nullptr);
    }
    llvm::verifyFunction(*function);

    ctx.variable_insert_point->eraseFromParent();
    ctx.variable_insert_point = nullptr;

    return nullptr;
}

llvm::Value* ResolvedIfExpr::codegen(Context& ctx) {
    llvm::Function* current_function = ctx.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* if_block = llvm::BasicBlock::Create(ctx.context, "if.true");
    llvm::BasicBlock* cont_block = llvm::BasicBlock::Create(ctx.context, "if.cont"); // Exits the if

    llvm::BasicBlock* else_block = cont_block;
    if (else_body) {
        else_block = llvm::BasicBlock::Create(ctx.context, "if.else");
    }

    llvm::Value* condition_code = condition->codegen(ctx);

    llvm::Value* boolean = nullptr;
    if (condition->type == Type::int64) {
        boolean = ctx.builder.CreateICmpNE(condition_code, llvm::ConstantInt::get(ctx.context, llvm::APInt{1, 0, true}));
    } else if (condition->type == Type::float64) {
        boolean = ctx.builder.CreateFCmpUNE(condition_code, llvm::ConstantFP::get(ctx.context, llvm::APFloat{0.0}));
    }

    ctx.builder.CreateCondBr(boolean, if_block, else_block);

    if_block->insertInto(current_function);
    ctx.builder.SetInsertPoint(if_block);
    llvm::Value* body_value = body->codegen(ctx, false);
    ctx.builder.CreateBr(cont_block);

    llvm::Value* else_value = nullptr;

    if (else_body) {
        else_block->insertInto(current_function);
        ctx.builder.SetInsertPoint(else_block);
        else_value = else_body->codegen(ctx, false);
        ctx.builder.CreateBr(cont_block);

        // REQUIRED to get up to date information about BasicBlocks!!! (e.g nested if-exprs)
        else_block = ctx.builder.GetInsertBlock();
    }

    cont_block->insertInto(current_function);
    ctx.builder.SetInsertPoint(cont_block);

    // If if-exprs actually return something, add a PHI node
    if (type != Type::void_) {
        llvm::PHINode* node = ctx.builder.CreatePHI(ctx.llvm_types.at(type), 2, "if.tmp");
        node->addIncoming(body_value, if_block);
        node->addIncoming(else_value, else_block);
        return node;
    }

    return nullptr;
}

llvm::Value* ResolvedOmg::codegen(Context& /*ctx*/) {
    std::cerr << "NOT IMPLEMENTED yet (OmgAST)\n";
    return nullptr;
}

llvm::Value* ResolvedExprStmt::codegen(Context& ctx) {
    return expr->codegen(ctx);
}

llvm::Value* ResolvedUnaryExpr::codegen(Context& ctx) {
    llvm::Value* expr_code = expr->codegen(ctx);
    if (!expr_code) {
        return nullptr;
    }

    if (op == TokenType::SUB) {
        if (type == Type::int64) {
            return ctx.builder.CreateNeg(expr_code);
        } else if (type == Type::float64) {
            return ctx.builder.CreateFNeg(expr_code);
        }
    }

    std::cerr << "NOT IMPLEMENTED YET (UnaryExprAST)\n";
    return nullptr;
}

llvm::Value* ResolvedBinaryExpr::codegen(Context& ctx) {
    llvm::Value* lhs_code = lhs->codegen(ctx);
    llvm::Value* rhs_code = rhs->codegen(ctx);
    if (!lhs_code || !rhs_code) {
        return nullptr;
    }

    switch (op) {
        // TODO: Add type system (wow)
        case TokenType::ADD:
            if (type == Type::int64) {
                return ctx.builder.CreateAdd(lhs_code, rhs_code);
            } else if (type == Type::float64) {
                return ctx.builder.CreateFAdd(lhs_code, rhs_code);
            }
            break;
        case TokenType::SUB:
            if (type == Type::int64) {
                return ctx.builder.CreateSub(lhs_code, rhs_code);
            } else if (type == Type::float64) {
                return ctx.builder.CreateFSub(lhs_code, rhs_code);
            }
            break;
        case TokenType::MUL:
            if (type == Type::int64) {
                return ctx.builder.CreateMul(lhs_code, rhs_code);
            } else if (type == Type::float64) {
                return ctx.builder.CreateFMul(lhs_code, rhs_code);
            }
            break;
        case TokenType::GREATER_THAN:
            if (type == Type::int64) {
                return ctx.builder.CreateICmpSGT(
                    lhs_code, rhs_code); // TODO: ICmpSGT Is only for I-nteger Cmp-arison with S-igned G-reater T-han
            } else if (type == Type::float64) {
                auto* comparison = ctx.builder.CreateFCmpUGT(lhs_code, rhs_code);
                return ctx.builder.CreateUIToFP(comparison, llvm::Type::getDoubleTy(ctx.context));
            }
            break;
        case TokenType::LESS_THAN:
            if (type == Type::int64) {
                return ctx.builder.CreateICmpSLT(
                    lhs_code, rhs_code); // TODO: ICmpSGT Is only for I-nteger Cmp-arison with S-igned L-ess T-han
            } else if (type == Type::float64) {
                auto* comparison = ctx.builder.CreateFCmpULT(lhs_code, rhs_code);
                return ctx.builder.CreateUIToFP(comparison, llvm::Type::getDoubleTy(ctx.context));
            }
            break;
        case TokenType::EQUAL:
            return ctx.builder.CreateICmpEQ(lhs_code, rhs_code);
        default:
            std::cerr << "NOT IMPLEMENTED YET (BinaryExprAST)\n";
    }
    return nullptr;
}

llvm::Value* ResolvedCall::codegen(Context& ctx) {
    llvm::Function* function = ctx.module->getFunction(callee->name);
    if (!function) {
        std::cout << "No function named '" + callee->name + "'\n";
        return nullptr;
    }

    size_t expected_num_args = function->arg_size();
    if (expected_num_args != arguments.size()) {
        // "Expected x argument(s) in call to function sussy, got y"
        std::cout << "Expected " + std::to_string(expected_num_args) + " argument" +
                         (expected_num_args != 1 ? "s " : " ") + "in call to function '" + callee->name + "', got " +
                         std::to_string(arguments.size())
                  << '\n';
        return nullptr;
    }

    std::vector<llvm::Value*> argument_values;
    for (auto&& arg : arguments) {
        llvm::Value* value = arg->codegen(ctx);
        argument_values.emplace_back(value);
        if (!argument_values.back()) {
            return nullptr;
        }
    }

    return ctx.builder.CreateCall(function, argument_values);
}

llvm::Value* ResolvedPrimitive::codegen(Context& ctx) {
    switch (value_type) {
        case ValueType::INT64:
            // std::cout << "Int\n";
            return llvm::ConstantInt::get(ctx.context, llvm::APInt{64, static_cast<uint64_t>(int64), true});
        case ValueType::UINT64:
            // std::cout << "Uint\n";
            return llvm::ConstantInt::get(ctx.context, llvm::APInt{64, uint64, false});
        case ValueType::FLOAT64:
            // std::cout << "Float\n";
            return llvm::ConstantFP::get(ctx.context, llvm::APFloat{float64});
        default:
            // std::cout << "L\n";
            return nullptr;
    }
}

llvm::Value* ResolvedVariable::codegen(Context& ctx) {
    llvm::Value* value = ctx.named_values[declaration];
    if (!value) {
        std::cout << "Unknown variable \"" + declaration->name + "\"" + '\n';
        return nullptr;
    }

    if (!dynamic_cast<ResolvedParamDeclare*>(declaration)) {
        return ctx.load_value(value, ctx.llvm_types.at(type));
    }
    return value;
}

llvm::Value* ResolvedAssignment::codegen(Context& ctx) {
    return ctx.builder.CreateStore(expr->codegen(ctx), ctx.named_values[variable->declaration]);
}

llvm::Value* ResolvedWhile::codegen(Context& ctx) {
    llvm::Function* current_function = ctx.builder.GetInsertBlock()->getParent();

    auto *cond = llvm::BasicBlock::Create(ctx.context, "while.cond", current_function);
    auto *body_block = llvm::BasicBlock::Create(ctx.context, "while.body", current_function);
    auto *exit = llvm::BasicBlock::Create(ctx.context, "while.exit", current_function);

    ctx.builder.CreateBr(cond);

    ctx.builder.SetInsertPoint(cond);
    llvm::Value* condition_code = condition->codegen(ctx);
    
    llvm::Value* boolean = nullptr;
    if (condition->type == Type::int64) {
        boolean = ctx.builder.CreateICmpNE(condition_code, llvm::ConstantInt::get(ctx.context, llvm::APInt{1, 0, true}));
    } else if (condition->type == Type::float64) {
        boolean = ctx.builder.CreateFCmpUNE(condition_code, llvm::ConstantFP::get(ctx.context, llvm::APFloat{0.0}));
    }
    ctx.builder.CreateCondBr(boolean, body_block, exit);

    ctx.builder.SetInsertPoint(body_block);
    body->codegen(ctx);
    ctx.builder.CreateBr(cond); // Goes back to cond

    ctx.builder.SetInsertPoint(exit);
    return nullptr;
}
