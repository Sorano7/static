#define CUT_IMPL
#include "include/cut.h"

int main(int argc, char **argv)
{
    cut_build_init();

    CutUnit app;
    cut_unit_init(&app, "app", CUT_UNIT_EXE);
    cut_unit_sources(&app, "src/main.c");
    cut_unit_includes(&app, "include");
    cut_unit_flags(&app, "-g", "-Wall", "-Wextra", "-Wno-override-init");

    cut_build_add(&app);
    return cut_build_run(argc, argv);
}
