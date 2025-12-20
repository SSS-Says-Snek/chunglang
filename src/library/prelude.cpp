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

bool window_should_close() {
    return WindowShouldClose();
}

void begin_drawing() {
    BeginDrawing();
}

void clear_background() {
    ClearBackground(BLACK);
}

void draw_circle(int64_t x, int64_t y, int64_t radius, int64_t r, int64_t g, int64_t b) {
    DrawCircle(x, y, radius, Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 255});
}

void draw_rectangle(int64_t x, int64_t y, int64_t width, int64_t height, int64_t r, int64_t g, int64_t b) {
    DrawRectangle(x, y, width, height, Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 255});
}

void draw_line(int64_t x, int64_t y, int64_t end_x, int64_t end_y, int64_t r, int64_t g, int64_t b) {
    DrawLine(x, y, end_x, end_y, Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 255});
}

void draw_number(int64_t x, int64_t y, int64_t number, int64_t font_size) {
    DrawText(TextFormat("%i", number), x, y, font_size, WHITE);
}

bool is_key_pressed(int64_t key) {
    return IsKeyDown(key);
}

void end_drawing() {
    EndDrawing();
}

void close_window() {
    CloseWindow();
}
}
