#include <stdio.h>
#include <raylib.h>
#include "editor.h"

#define CUT_IMPL
#include "cut.h"

constexpr int window_w = 800;
constexpr int window_h = 600;

constexpr float line_spacing = 2;
constexpr float font_size_step = 2;

float font_size = 28;

#define key_pressed_or_held(key) (IsKeyPressed(key) || IsKeyPressedRepeat(key))

static void handle_keys(Editor *ed)
{
    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    int key = GetCharPressed();
    while (key > 0)
    {
        if ((key >= 32) && (key <= 125))
            editor_do(ed, ACTION_INSERT_CHR, .chr=(char)key);
        key = GetCharPressed();
    }

    if (key_pressed_or_held(KEY_BACKSPACE))
    {
        if (shift)
            editor_do(ed, ACTION_REMOVE_LINE);
        else
            editor_do(ed, ACTION_DELETE);
    }

    if (key_pressed_or_held(KEY_ENTER))
    {
        if (shift)     editor_do(ed, ACTION_NEWLINE_ABOVE);
        else if (ctrl) editor_do(ed, ACTION_NEWLINE_BELOW);
        else           editor_do(ed, ACTION_SPLIT_LINE);
    }

    if (IsKeyPressed(KEY_TAB)) editor_do(ed, ACTION_INSERT_STR, .str="    ");

    if (key_pressed_or_held(KEY_LEFT))  editor_do(ed, ACTION_CURSOR_MOVE, .dir=DIR_LEFT);
    if (key_pressed_or_held(KEY_DOWN))  editor_do(ed, ACTION_CURSOR_MOVE, .dir=DIR_DOWN);
    if (key_pressed_or_held(KEY_UP))    editor_do(ed, ACTION_CURSOR_MOVE, .dir=DIR_UP);
    if (key_pressed_or_held(KEY_RIGHT)) editor_do(ed, ACTION_CURSOR_MOVE, .dir=DIR_RIGHT);

    if (ctrl)
    {
        if (key_pressed_or_held(KEY_EQUAL)) font_size += font_size_step;
        if (key_pressed_or_held(KEY_MINUS)) font_size -= font_size_step;
    }
}

static void render_buffer(Editor *ed, Font font)
{
    Vector2 line_size = MeasureTextEx(font, "A", font_size, 0);
    float line_h = line_spacing + line_size.y;

    ClearBackground(BLACK);

    for (size_t i = 0; i < editor_line_count(ed); i++)
    {
        const char *line = editor_getline(ed, i);
        Vector2 pos = {10, i * line_h};
        DrawTextEx(font, line, pos, font_size, 0, RAYWHITE);

        size_t row = editor_row(ed);
        size_t col = editor_col(ed);
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
}

int main(void)
{
    InitWindow(window_w, window_h, "Editor");
    SetExitKey(KEY_NULL);

    Font font = LoadFontEx("res/IosevkaWide-Regular.ttf", 64, NULL, 0);
    Editor *ed = editor_create();

    while (!WindowShouldClose())
    {
        handle_keys(ed);

        BeginDrawing();
            render_buffer(ed, font);
        EndDrawing();
    }

    editor_free(ed);
    CloseWindow();
    return 0;
}
