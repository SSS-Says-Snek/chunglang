; ModuleID = '<module sus>'
source_filename = "<module sus>"

@0 = private unnamed_addr constant [4 x i8] c"gurt", align 1

declare void @print(i64)

declare void @print_char(i64)

declare void @print_float_64(double)

declare void @print_string({ ptr, i64 })

declare void @init_window(i64, i64)

declare void @set_target_fps(i64)

declare i1 @window_should_close()

declare void @begin_drawing()

declare void @clear_background()

declare void @draw_circle(i64, i64, i64, i64, i64, i64)

declare void @draw_rectangle(i64, i64, i64, i64, i64, i64, i64)

declare void @draw_line(i64, i64, i64, i64, i64, i64, i64)

declare void @draw_number(i64, i64, i64, i64)

declare i1 @is_key_pressed(i64)

declare void @end_drawing()

declare void @close_window()

define void @main() {
entry:
  %e = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @0, i64 4 }, ptr %e, align 8
  %0 = load { ptr, i64 }, ptr %e, align 8
  call void @print_string(ptr %e)
  ret void
}
