#include "chung/context.hpp"

Context::Context():
    context{llvm::LLVMContext()}, 
    builder{llvm::IRBuilder<>(context)},
    module{std::make_unique<llvm::Module>("<module sus>", context)} {
    declared_types = {
        {"uint64", Type::tuint64},
        {"int64", Type::tint64},
        {"float64", Type::tfloat64},
        {"string", Type::tstring}
    };
    llvm_types = {
        {Type::tuint64, llvm::Type::getInt64Ty(context)},
        {Type::tint64, llvm::Type::getInt64Ty(context)},
        {Type::tfloat64, llvm::Type::getDoubleTy(context)},
        // {Type::tstring, builder.getInt8PtrTy()}
    };
}

Type& Context::get_type(const std::string& type_identifier) {
    auto result = declared_types.find(type_identifier);
    if (result == declared_types.end()) {
        return Type::tinvalid;
    }
    return result->second;
}