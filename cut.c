#define CUT_IMPL
#include "include/cut.h"

#ifdef _WIN32
    #define RAYLIB_DEPS "raylib" "opengl32" "gdi32" "winmm" "user32" "shell32"
#else
    #define RAYLIB_DEPS "raylib", "GL", "m", "pthread", "dl", "rt", "X11"
#endif

void config(CutUnit *u)
{
    cut_unit_sources(u, "src/main.c");
    cut_unit_sources(u,"src/editor.c", "src/buffer.c");

    cut_unit_includes(u, "include");
    cut_unit_defines(u, "_DEFAULT_SOURCE");
    cut_unit_flags(u, "-std=c23", "-g", "-Wall", "-Wextra", "-Wno-override-init");

    cut_unit_libs(u, RAYLIB_DEPS);
}

int main(int argc, char **argv)
{
    cut_build_init();

    CutUnit dev;
    cut_unit_init(&dev, "dev", CUT_UNIT_EXE);
    cut_unit_out_name(&dev, "static_dev");
    cut_unit_defines(&dev, "_DEV");
    config(&dev);

    CutUnit release;
    cut_unit_init(&release, "release", CUT_UNIT_EXE);
    cut_unit_out_name(&release, "static");
    config(&release);

    cut_build_add(&dev, &release);
    return cut_build_run(argc, argv);
}
