#include "chung/library/setup_prelude.hpp"

void setup_function(Context& ctx, const std::string& name, const std::vector<std::pair<std::string, llvm::Type*>>& params,
                    llvm::Type* return_type) {
    std::vector<llvm::Type*> params_type;
    params_type.reserve(params.size());
    for (const auto& param : params) {
        params_type.push_back(param.second);
    }
    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, params_type, false);
    llvm::Function* func =
        llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, ctx.module.get());

    size_t i = 0;
    for (auto& arg : func->args()) {
        arg.setName(params[i].first);
        i++;
    }
    
    ctx.c_builtins.push_back(name);
}

void setup_prelude(Context& ctx) {
    llvm::Type* int64_type = llvm::Type::getInt64Ty(ctx.context);
    llvm::Type* void_type = llvm::Type::getVoidTy(ctx.context);
    llvm::Type* bool_type = llvm::Type::getInt1Ty(ctx.context);

    setup_function(ctx, "print", {{"value", llvm::Type::getInt64Ty(ctx.context)}}, llvm::Type::getVoidTy(ctx.context));
    setup_function(ctx, "print_char", {{"value", llvm::Type::getInt64Ty(ctx.context)}}, llvm::Type::getVoidTy(ctx.context));
    setup_function(ctx, "print_float64", {{"value", llvm::Type::getDoubleTy(ctx.context)}}, llvm::Type::getVoidTy(ctx.context));
    setup_function(ctx, "print_string", {{"value", ctx.llvm_types.at(Type::string)}}, llvm::Type::getVoidTy(ctx.context));

    // Raylib
    setup_function(ctx, "init_window", {{"width", int64_type}, {"height", int64_type}}, void_type);
    setup_function(ctx, "set_target_fps", {{"fps", int64_type}}, void_type);
    setup_function(ctx, "window_should_close", {}, bool_type);
    setup_function(ctx, "begin_drawing", {}, void_type);
    setup_function(ctx, "clear_background", {}, void_type);
    setup_function(ctx, "draw_circle", {{"x", int64_type}, {"y", int64_type}, {"radius", int64_type}, {"r", int64_type}, {"g", int64_type}, {"b", int64_type}}, void_type);
    setup_function(ctx, "draw_rectangle", {{"x", int64_type}, {"y", int64_type}, {"width", int64_type}, {"height", int64_type}, {"r", int64_type}, {"g", int64_type}, {"b", int64_type}}, void_type);
    setup_function(ctx, "draw_line", {{"x", int64_type}, {"y", int64_type}, {"end_x", int64_type}, {"end_y", int64_type}, {"r", int64_type}, {"g", int64_type}, {"b", int64_type}}, void_type);
    setup_function(ctx, "draw_number", {{"x", int64_type}, {"y", int64_type}, {"number", int64_type}, {"font_size", int64_type}}, void_type);
    setup_function(ctx, "is_key_pressed", {{"key", int64_type}}, bool_type);
    setup_function(ctx, "end_drawing", {}, void_type);
    setup_function(ctx, "close_window", {}, void_type);
}
