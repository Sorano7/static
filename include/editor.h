#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include <raylib.h>

typedef struct
{
    Font font;
    float font_size;
    Color bg;
    Color fg;
    bool text_effect;
    bool text_wrap;
} RenderOpt;

typedef struct
{
    Buffer *buf;
    float input_rate;
    RenderOpt opt;
} Editor;


void editor_init(Editor *ed, Buffer *buf);
void editor_free(Editor *ed);
void editor_default_opt(Editor *ed);
void editor_set_opt(Editor *ed, const RenderOpt *opt);

void editor_update(Editor *ed);
void editor_render(Editor *ed);

#endif
