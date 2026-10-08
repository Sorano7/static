#define CUT_IMPL
#include "include/cut.h"

#ifdef _WIN32
    #define RAYLIB_DEPS "raylib" "opengl32" "gdi32" "winmm" "user32" "shell32"
#else
    #define RAYLIB_DEPS "raylib", "GL", "m", "pthread", "dl", "rt", "X11"
#endif

int main(int argc, char **argv)
{
    cut_build_init();

    CutUnit app;
    cut_unit_init(&app, "app", CUT_UNIT_EXE);
    cut_unit_sources(&app, "src/main.c");
    cut_unit_sources(&app, "src/editor.c");

    cut_unit_includes(&app, "include");
    cut_unit_defines(&app, "_DEFAULT_SOURCE")
    cut_unit_flags(&app, "-std=c23", "-g", "-Wall", "-Wextra", "-Wno-override-init");

    cut_unit_libs(&app, RAYLIB_DEPS);

    cut_build_add(&app);
    return cut_build_run(argc, argv);
}
