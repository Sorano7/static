#ifndef EDITOR_H
#define EDITOR_H

#include <stdlib.h>

typedef struct Editor Editor;

Editor *ed_create(void);
void ed_free(Editor *ed);

size_t ed_get_row(Editor *ed);
size_t ed_get_col(Editor *ed);

typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} Direction;

void ed_move(Editor *ed, Direction dir);

void ed_insert_char(Editor *ed, char c);
void ed_insert_str(Editor *ed, const char *str);

void ed_linebreak(Editor *ed);
void ed_newline(Editor *ed, bool below);
void ed_remove_line(Editor *ed);

void ed_backspace(Editor *ed);

size_t ed_line_count(Editor *ed);
const char *ed_getline(Editor *ed, size_t row);

#endif
