#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include <raylib.h>

typedef struct
{
    Font font;
    float font_size;

    Color bg;
    Color mantle;

    Color text;
    Color overlay;

    bool text_effect;
    bool text_wrap;
} RenderOpt;

void default_render_opt(RenderOpt *opt);

typedef enum
{
    MODE_EDIT,
    MODE_PROMPT,
} EditorMode;

typedef struct Editor Editor;

typedef void (*PromptCallBack)(Editor *, StringView, void *);

typedef struct
{
    Buffer *buf;
    PromptCallBack cb;
    void *ud;
} Prompt;

typedef struct Editor
{
    Buffer *buf;
    Prompt prompt;

    RenderOpt *opt;

    float input_rate;
    EditorMode mode;

    String msg;
} Editor;

// Initialize an editor optionally with a buffer.
void editor_init(Editor *ed, Buffer *buf);

// Free the editor.
void editor_free(Editor *ed);

// Set options for the editor.
void editor_set_opt(Editor *ed, RenderOpt *opt);

// Update the editor.
void editor_update(Editor *ed);

// Render the editor.
void editor_render(Editor *ed);

#endif
