#include "chung/library/setup_prelude.hpp"

void setup_prelude(Context& ctx) {
    // print
    std::vector<llvm::Type*> print_params{llvm::Type::getInt64Ty(ctx.context)};
    llvm::Type* print_return_type = llvm::Type::getVoidTy(ctx.context);
    llvm::FunctionType* print_func_type = llvm::FunctionType::get(print_return_type, print_params, false);
    llvm::Function* print_func =
        llvm::Function::Create(print_func_type, llvm::Function::ExternalLinkage, "print", ctx.module.get());

    for (auto& arg : print_func->args()) {
        arg.setName("value");
    }

    // print_char
    std::vector<llvm::Type*> print_char_params{llvm::Type::getInt64Ty(ctx.context)};
    llvm::Type* print_char_return_type = llvm::Type::getVoidTy(ctx.context);
    llvm::FunctionType* print_char_func_type = llvm::FunctionType::get(print_return_type, print_params, false);
    llvm::Function* print_char_func =
        llvm::Function::Create(print_func_type, llvm::Function::ExternalLinkage, "print_char", ctx.module.get());

    for (auto& arg : print_char_func->args()) {
        arg.setName("value");
    }
}
