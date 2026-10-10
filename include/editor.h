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
    String msg;
} Editor;

// Initialize an editor optionally with a buffer.
void editor_init(Editor *ed, Buffer *buf);

// Free the editor.
void editor_free(Editor *ed);

// Set default options for the editor.
void editor_default_opt(Editor *ed);

// Set options for the editor.
void editor_set_opt(Editor *ed, const RenderOpt *opt);

// Update the editor.
void editor_update(Editor *ed);

// Render the editor.
void editor_render(Editor *ed);

#endif
