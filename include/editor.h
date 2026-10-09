#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include <raylib.h>

typedef struct Edtior 
{
    Buffer *buf;
    float font_size;
    float input_rate;
} Editor;

typedef struct
{
    Font font;
    Color bg;
    Color fg;
    bool text_effect;
} RenderOpt;

void editor_init(Editor *ed, Buffer *buf);
void editor_free(Editor *ed);

void editor_update(Editor *ed);
void editor_render(Editor *ed, RenderOpt *opt);

#endif
