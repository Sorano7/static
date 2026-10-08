#ifndef EDITOR_H
#define EDITOR_H

#include <stdlib.h>

typedef struct Editor Editor;

Editor *editor_create(void);
void editor_free(Editor *ed);

size_t editor_row(Editor *ed);
size_t editor_col(Editor *ed);

size_t editor_line_count(Editor *ed);
const char *editor_getline(Editor *ed, size_t row);

typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} Direction;

typedef enum
{
    ACTION_INSERT_CHR,
    ACTION_INSERT_STR,
    ACTION_DELETE,

    ACTION_REMOVE_LINE,
    ACTION_NEWLINE_BELOW,
    ACTION_NEWLINE_ABOVE,
    ACTION_SPLIT_LINE,

    ACTION_CURSOR_MOVE,
} ActionKind;

typedef struct
{
    ActionKind kind;
    union
    {
        Direction dir;
        char chr;
        const char *str;
    };
} Action;

void editor_handle_action(Editor *ed, Action action);
#define editor_do(_ed, _act_kind, ...) editor_handle_action((_ed), \
        (Action){(_act_kind), .dir=DIR_NONE, __VA_ARGS__})

#endif
