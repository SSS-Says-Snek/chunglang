#include "chung/library/prelude.hpp"

extern "C" {
#include <cstdio>
#include <cinttypes>
#include <raylib.h>

void print(int64_t int64) {
    printf("%" PRId64 "\n", int64);
}

void print_char(int64_t int64) {
    printf("%c", (char)int64);
}

void print_float64(double float64) {
    printf("%f\n", float64);
}

// Raylib
void init_window(int64_t width, int64_t height) {
    InitWindow(width, height, "My Chunglang Game!");
}

void set_target_fps(int64_t fps) {
    SetTargetFPS(fps);
}

int64_t window_should_close() {
    return WindowShouldClose();
}

void begin_drawing() {
    BeginDrawing();
}

void clear_background() {
    ClearBackground(BLACK);
}

void draw_circle(int64_t x, int64_t y, int64_t radius) {
    DrawCircle(x, y, radius, WHITE);
}

void end_drawing() {
    EndDrawing();
}

void close_window() {
    CloseWindow();
}
}
