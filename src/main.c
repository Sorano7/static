#include <raylib.h>
#include "editor.h"

#define CUT_IMPL
#include "cut.h"

constexpr int window_w = 800;
constexpr int window_h = 600;

int main(void)
{
    InitWindow(window_w, window_h, "static");
    SetExitKey(KEY_NULL);

    Font font = LoadFontEx("res/IosevkaWide-Regular.ttf", 64, NULL, 0);
    Editor ed;
    editor_init(&ed, nullptr);

    RenderOpt opt = {.font=font};

    while (!WindowShouldClose())
    {
        editor_update(&ed);

        BeginDrawing();

            editor_render(&ed, &opt);

        EndDrawing();
    }

    editor_free(&ed);
    CloseWindow();
    return 0;
}
