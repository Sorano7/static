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

    Editor ed;
    editor_init(&ed, nullptr);

    RenderOpt opt;
    default_render_opt(&opt);

    editor_set_opt(&ed, &opt);

    while (!WindowShouldClose())
    {
        editor_update(&ed);

        BeginDrawing();

            editor_render(&ed);

        EndDrawing();
    }

    editor_free(&ed);
    CloseWindow();
    return 0;
}
