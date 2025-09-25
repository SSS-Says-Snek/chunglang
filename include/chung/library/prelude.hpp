#pragma once

#include "chung/context.hpp"

extern "C" {
    void print(int64_t int64);
}

void setup_prelude(Context& ctx);