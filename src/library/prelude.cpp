#include "chung/library/prelude.hpp"

extern "C" {
#include <cstdio>
#include <cinttypes>

void print(int64_t int64) {
    printf("%" PRId64 "\n", int64);
}
}
