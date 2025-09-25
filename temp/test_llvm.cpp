#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <stdio.h>
#include <iostream>

using namespace llvm;

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT int printchar(int c) {
    putc(c, stdout);
    return 0;
}

int main() {
    auto context = std::make_unique<LLVMContext>();
    IRBuilder<> builder(*context);

    auto module = std::make_unique<Module>("hello", *context);

    // build a 'main' function
    auto i32 = builder.getInt32Ty();
    auto prototype = FunctionType::get(i32, false);
    Function *main_fn = Function::Create(prototype, Function::ExternalLinkage, "main", module.get());
    BasicBlock *body = BasicBlock::Create(*context, "body", main_fn);
    builder.SetInsertPoint(body);

    // use libc's printf function
    auto i8p = builder.getInt8PtrTy();
    auto i8 = builder.getInt8Ty();
    auto printf_prototype = FunctionType::get(i8p, true);
    auto putchar_prototype = FunctionType::get(i32, false);
    auto printf_fn = Function::Create(printf_prototype, Function::ExternalLinkage, "printf", module.get());
    auto fputc_fn = Function::Create(putchar_prototype, Function::ExternalLinkage, "printchar", module.get());

    // call printf with our string
    auto format_str = builder.CreateGlobalStringPtr("hello world\n");
    builder.CreateCall(printf_fn, { format_str });
    // builder.CreateCall(printf_fn, {format_str});

    // return 0 from main
    auto e = ConstantInt::get(*context, APInt{32, 42, true});
    auto f = ConstantInt::get(*context, APInt{32, 42, true});
    auto g = ConstantInt::get(*context, APInt{32, 42, true});

    Value* lhs = builder.CreateAdd(e, f, "addtmp");
    Value* expr = builder.CreateAdd(lhs, g, "addtmp");
    builder.CreateCall(fputc_fn, {expr}, "calltemp");
    builder.CreateRet(ConstantInt::get(i32, 0));

    // if you want to print the LLVM IR:
    module->print(llvm::outs(), nullptr);

    // execute it!
    ExecutionEngine *executionEngine = EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::Interpreter).create();
    Function *main = executionEngine->FindFunctionNamed(StringRef("main"));
    auto result = executionEngine->runFunction(main, {});

    // return the result
    return result.IntVal.getLimitedValue();
}