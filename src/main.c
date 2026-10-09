#include <raylib.h>
#include "editor.h"

#define CUT_IMPL
#include "cut.h"

constexpr int window_w = 800;
constexpr int window_h = 600;

int main(void)
{
    srand(time(NULL));

    InitWindow(window_w, window_h, "static");
    SetExitKey(KEY_NULL);

    Font font = LoadFontEx("res/IosevkaWide-Regular.ttf", 64, NULL, 0);
    Editor ed;
    editor_init(&ed, nullptr);

    RenderOpt opt = {
        .font        = font,
        .fg          = GetColor(0xd5dde3ff),
        .bg          = GetColor(0x181e29ff),
        .text_effect = true,
    };

    while (!WindowShouldClose())
    {

        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

        editor_update(&ed);

        if (ctrl) 
        {
            if (IsKeyPressed(KEY_L))
            {
                Color prev_fg = opt.fg;
                opt.fg = opt.bg;
                opt.bg = prev_fg;
            }

            #ifdef _DEV
                if (IsKeyPressed(KEY_P)) opt.text_effect = !opt.text_effect;
            #endif
        }

        BeginDrawing();

            editor_render(&ed, &opt);

        EndDrawing();
    }

    editor_free(&ed);
    CloseWindow();
    return 0;
}
