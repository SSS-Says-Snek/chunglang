#include "chung/resolved_ast.hpp"
#include <iostream>
#include <llvm/IR/BasicBlock.h>

llvm::Value* ResolvedVarDeclare::codegen(Context& ctx) {
    // For now
    return expr->codegen(ctx);
}

llvm::Value* ResolvedFunction::codegen(Context& ctx) {
    std::vector<llvm::Type*> parameter_types;
    for (auto& parameter : parameters) {
        parameter_types.push_back(ctx.llvm_types.at(parameter.type));
    }

    // FOR NOW RET VOID
    llvm::FunctionType* function_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx.context), parameter_types, false);
    llvm::Function* function =
        llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name, ctx.module.get());

    ctx.named_values.clear();

    // Set parameter names
    size_t i = 0;
    for (auto& function_parameter : function->args()) {
        auto& parameter_name = parameters[i++].name;
        function_parameter.setName(parameter_name);
        ctx.named_values[parameter_name] = &function_parameter;
    }

    // Basic Block
    llvm::BasicBlock* function_block = llvm::BasicBlock::Create(ctx.context, "entry", function);
    ctx.builder.SetInsertPoint(function_block);

    for (auto& stmt : body) {
        stmt->codegen(ctx);
    }

    // Void FOR NOW
    ctx.builder.CreateRet(nullptr);
    llvm::verifyFunction(*function);

    return nullptr;
}

llvm::Value* ResolvedIfExpr::codegen(Context& ctx) {
    llvm::Function* current_function = ctx.builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* if_block = llvm::BasicBlock::Create(ctx.context, "if.true");
    llvm::BasicBlock* cont_block = llvm::BasicBlock::Create(ctx.context, "if.cont"); // Exits the if

    llvm::BasicBlock* else_block = cont_block;
    if (!else_body.empty()) {
        else_block = llvm::BasicBlock::Create(ctx.context, "if.else");
    }

    llvm::Value* condition_code = condition->codegen(ctx);

    llvm::Value* boolean =
        ctx.builder.CreateICmpNE(condition_code, llvm::ConstantInt::get(ctx.context, llvm::APInt{64, 0, true}));

    ctx.builder.CreateCondBr(boolean, if_block, else_block);

    if_block->insertInto(current_function);
    ctx.builder.SetInsertPoint(if_block);
    for (auto& stmt : body) {
        stmt->codegen(ctx);
    }
    ctx.builder.CreateBr(cont_block);

    if (!else_body.empty()) {
        else_block->insertInto(current_function);
        ctx.builder.SetInsertPoint(else_block);
        for (auto& stmt : else_body) {
            stmt->codegen(ctx);
        }
        ctx.builder.CreateBr(cont_block);
    }

    cont_block->insertInto(current_function);
    ctx.builder.SetInsertPoint(cont_block);
    return nullptr;
}

llvm::Value* ResolvedOmg::codegen(Context& ctx) {
    std::cerr << "NOT IMPLEMENTED yet (OmgAST)\n";
    return nullptr;
}

llvm::Value* ResolvedExprStmt::codegen(Context& ctx) {
    return expr->codegen(ctx);
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
            lhs_code->getType()->print(llvm::outs());
            return ctx.builder.CreateAdd(lhs_code, rhs_code);
        case TokenType::SUB:
            lhs_code->getType()->print(llvm::outs());
            return ctx.builder.CreateSub(lhs_code, rhs_code);
        case TokenType::MUL:
            lhs_code->getType()->print(llvm::outs());
            return ctx.builder.CreateMul(lhs_code, rhs_code);
        case TokenType::GREATER_THAN:
            lhs_code->getType()->print(llvm::outs());
            return ctx.builder.CreateICmpSGT(
                lhs_code, rhs_code); // TODO: ICmpSGT Is only for I-nteger Cmp-arison with S-igned G-reater T-han
        default:
            std::cerr << "NOT IMPLEMENTED YET (BinaryExprAST)\n";
    }
    return nullptr;
}

llvm::Value* ResolvedCall::codegen(Context& ctx) {
    llvm::Function* function = ctx.module->getFunction(callee);
    if (!function) {
        std::cout << "No function named '" + callee + "'\n";
        return nullptr;
    }

    size_t expected_num_args = function->arg_size();
    if (expected_num_args != arguments.size()) {
        // "Expected x argument(s) in call to function sussy, got y"
        std::cout << "Expected " + std::to_string(expected_num_args) + " argument" +
                         (expected_num_args != 1 ? "s " : " ") + "in call to function '" + callee + "', got " +
                         std::to_string(arguments.size())
                  << '\n';
        return nullptr;
    }

    std::vector<llvm::Value*> argument_values;
    for (auto& arg : arguments) {
        argument_values.push_back(arg->codegen(ctx));
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
    llvm::Value* value = ctx.named_values[name];
    if (!value) {
        std::cout << "Unknown variable \"" + name + "\"";
        return nullptr;
    }
    return value;
}
