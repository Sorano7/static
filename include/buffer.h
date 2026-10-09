#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include "cut.h"

typedef struct Buffer Buffer;

Buffer *buf_create(void);
void buf_free(Buffer *buf);

size_t buf_row(Buffer *buf);
size_t buf_col(Buffer *buf);

size_t buf_line_count(Buffer *buf);
StringView buf_getline(Buffer *buf, size_t row);

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
} BufActionKind;

typedef struct
{
    BufActionKind kind;
    Direction dir;
    char chr;
    const char *str;
} BufAction;

void buf_handle_action(Buffer *buf, BufAction action);
#define buf_do(_buf, kind, ...) buf_handle_action((_buf), (BufAction){ \
            (kind), \
            .dir=DIR_NONE, .chr='\0', .str=nullptr, \
            __VA_ARGS__ \
        })

#endif
