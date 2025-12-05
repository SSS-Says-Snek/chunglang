#include "chung/context.hpp"

Context::Context()
    : context{llvm::LLVMContext()}, builder{llvm::IRBuilder<>(context)},
      module{std::make_unique<llvm::Module>("<module sus>", context)} {
    declared_types = {
        {"uint64", Type::uint64}, {"int64", Type::int64}, {"float64", Type::float64}, {"string", Type::string}};
    llvm_types = {
        {Type::uint64, llvm::Type::getInt64Ty(context)},
        {Type::int64, llvm::Type::getInt64Ty(context)},
        {Type::float64, llvm::Type::getDoubleTy(context)},
        // {Type::tstring, builder.getInt8PtrTy()}
    };
}

Type Context::get_type(const std::string& type_identifier) {
    auto result = declared_types.find(type_identifier);
    if (result == declared_types.end()) {
        return Type::user(type_identifier);
    }
    return result->second;
}
