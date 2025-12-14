#pragma once

#include <map>
#include <functional>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "chung/type.hpp"

class ResolvedDecl;
struct Context;
#include "chung/resolved_ast.hpp"

struct Context {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Instruction* variable_insert_point; // alloca
    std::unique_ptr<llvm::Module> module;
    std::map<ResolvedDecl*, llvm::Value*> named_values;
    std::map<std::string, Type> declared_types;
    std::map<std::reference_wrapper<const Type>, llvm::Type*, std::less<const Type>> llvm_types; // NOLINT

    Context();

    Type get_type(const std::string& type_identifier);

    llvm::AllocaInst* allocate_stack_variable(std::string_view name, llvm::Type* type);

    llvm::Value* load_value(llvm::Value* value, llvm::Type* type) {
        return builder.CreateLoad(type, value);
    }
};
