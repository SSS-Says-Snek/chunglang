#pragma once

#include "chung/context.hpp"

void setup_prelude(Context& ctx);
void setup_function(Context& ctx, const std::string& name, const std::vector<std::pair<std::string, llvm::Type*>>& params, const llvm::Type* return_type);
