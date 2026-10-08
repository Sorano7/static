#include <stdio.h>
#include <raylib.h>
#include "editor.h"

#define CUT_IMPL
#include "cut.h"

constexpr int window_w = 800;
constexpr int window_h = 600;

constexpr float line_spacing = 2;
constexpr float font_size_step = 2;

#define key_pressed_or_held(key) (IsKeyPressed(key) || IsKeyPressedRepeat(key))

int main(void)
{
    InitWindow(window_w, window_h, "Editor");
    SetExitKey(KEY_NULL);

    Font font = LoadFontEx("res/IosevkaWide-Regular.ttf", 64, NULL, 0);
    float font_size = 28;

    Editor *ed = ed_create();

    while (!WindowShouldClose())
    {
        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125))
                ed_insert_char(ed, (char)key);
            key = GetCharPressed();
        }

        if (key_pressed_or_held(KEY_BACKSPACE))
        {
            shift ? ed_remove_line(ed) : ed_backspace(ed);
        }

        if (key_pressed_or_held(KEY_ENTER))
        {
            if (shift)     ed_newline(ed, false);
            else if (ctrl) ed_newline(ed, true);
            else           ed_linebreak(ed);
        }

        if (IsKeyPressed(KEY_TAB)) ed_insert_str(ed, "    ");

        if (key_pressed_or_held(KEY_LEFT))  ed_move(ed, DIR_LEFT);
        if (key_pressed_or_held(KEY_DOWN))  ed_move(ed, DIR_DOWN);
        if (key_pressed_or_held(KEY_UP))    ed_move(ed, DIR_UP);
        if (key_pressed_or_held(KEY_RIGHT)) ed_move(ed, DIR_RIGHT);

        if (ctrl)
        {
            if (key_pressed_or_held(KEY_EQUAL)) font_size += font_size_step;
            if (key_pressed_or_held(KEY_MINUS)) font_size -= font_size_step;
        }

        BeginDrawing();
            Vector2 line_size = MeasureTextEx(font, "A", font_size, 0);
            float line_h = line_spacing + line_size.y;

            ClearBackground(BLACK);

            for (size_t i = 0; i < ed_line_count(ed); i++)
            {
                const char *line = ed_getline(ed, i);
                Vector2 pos = {10, i * line_h};
                DrawTextEx(font, line, pos, font_size, 0, RAYWHITE);

                size_t row = ed_get_row(ed);
                size_t col = ed_get_col(ed);
                if (i == row)
                {
                    Rectangle cursor = {
                        .x      = 10 + line_size.x * col,
                        .y      = row * line_h,
                        .width  = line_size.x,
                        .height = line_size.y,
                    };

                    DrawRectangleRec(cursor, RAYWHITE);
                    char cursor_char[2];
                    cursor_char[0] = *(line + col);
                    cursor_char[1] = '\0';

                    pos.x += line_size.x * col;

                    DrawTextEx(font, cursor_char, pos, font_size, 0, BLACK);
                }

            }
        EndDrawing();
    }

    ed_free(ed);
    CloseWindow();
    return 0;
}
