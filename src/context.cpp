#include "chung/context.hpp"

Context::Context()
    : context{llvm::LLVMContext()}, builder{llvm::IRBuilder<>(context)},
      module{std::make_unique<llvm::Module>("<module sus>", context)} {
    declared_types = {{"uint64", Type::uint64},
                      {"int64", Type::int64},
                      {"float64", Type::float64},
                      {"string", Type::string},
                      {"bool", Type::boolean}};
    llvm_types = {
        {Type::uint64, llvm::Type::getInt64Ty(context)},
        {Type::int64, llvm::Type::getInt64Ty(context)},
        {Type::float64, llvm::Type::getDoubleTy(context)},
        {Type::void_, llvm::Type::getVoidTy(context)},
        {Type::boolean, llvm::Type::getInt1Ty(context)},
        {Type::string,
         llvm::StructType::get(context, {llvm::PointerType::get(context, 0), llvm::Type::getInt64Ty(context)})}
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

llvm::AllocaInst* Context::allocate_stack_variable(std::string_view name, llvm::Type* type) {
    llvm::IRBuilder<> tmpBuilder(context);
    tmpBuilder.SetInsertPoint(variable_insert_point);

    return tmpBuilder.CreateAlloca(type, nullptr, name);
}
