#pragma once

#include <map>
#include <functional>

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

#include "chung/type.hpp"

struct Context {
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::Value*> named_values;
    std::map<std::string, Type&> declared_types;
    std::map<std::reference_wrapper<const Type>, llvm::Type*, std::less<const Type>> llvm_types;

    Context();

    Type& get_type(const std::string& type_identifier);
};