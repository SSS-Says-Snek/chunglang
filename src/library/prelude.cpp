#include "chung/library/prelude.hpp"

extern "C" {
    #include <cstdio>
    #include <cinttypes>

    void print(int64_t int64) {
        printf("%" PRId64 "\n", int64);
    }
}

void setup_prelude(Context& ctx) {
    // print
    std::vector<llvm::Type*> print_params{llvm::Type::getInt64Ty(ctx.context)};
    llvm::Type* print_return_type = llvm::Type::getVoidTy(ctx.context);
    llvm::FunctionType* print_func_type = llvm::FunctionType::get(print_return_type, print_params, false);
    llvm::Function* print_func = llvm::Function::Create(print_func_type, llvm::Function::ExternalLinkage, "print", ctx.module.get());
    
    for (auto& arg: print_func->args()) {
        arg.setName("value");
    }
}