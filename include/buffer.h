#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct Buffer
{
    LineList lines;
    size_t row, col;
} Buffer;

Buffer *buf_create(void);
void buf_free(Buffer *buf);

size_t buf_line_count(const Buffer *buf);
StringView buf_getline(const Buffer *buf, size_t row);

typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} Direction;

void buf_move_cursor(Buffer *buf, Direction dir);

void buf_insert_chr(Buffer *buf, char c);
void buf_insert_str(Buffer *buf, const char *str);
void buf_delete_chr(Buffer *buf);

void buf_split_line(Buffer *buf);
void buf_insert_line(Buffer *buf, Direction dir);
void buf_remove_line(Buffer *buf);

#endif
