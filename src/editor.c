#include "editor.h"
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct Editor
{
    LineList lines;
    size_t row, col;
} Editor;

static void alloc_line(Editor *ed)
{
    String *line = malloc(sizeof(String));
    str_init(line);
    da_append(&ed->lines, line);
}

Editor *ed_create(void)
{
    Editor *ed = malloc(sizeof(Editor));
    da_init(&ed->lines);
    ed->row = 0;
    ed->col = 0;
    alloc_line(ed);
    return ed;
}

void ed_free(Editor *ed)
{
    if (!ed) return;
    DA_FOREACH(&ed->lines, String *, line)
        str_free(*line);
    da_free(&ed->lines);
}

size_t ed_get_row(Editor *ed)
{
    return ed->row;
}

size_t ed_get_col(Editor *ed)
{
    return ed->col;
}

static String *current_line(Editor *ed)
{
    return ed->lines.data[ed->row];
}

static void ed_clamp_col(Editor *ed)
{
    size_t line_len = current_line(ed)->len;
    if (ed->col > line_len) ed->col = line_len;
}

void ed_move(Editor *ed, Direction dir)
{
    switch (dir)
    {
        case DIR_UP:
            if (ed->row > 0) ed->row--;
            ed_clamp_col(ed);
            break;

        case DIR_DOWN:
            if (ed->row < ed->lines.len - 1) ed->row++;
            ed_clamp_col(ed);
            break;

        case DIR_LEFT:
            if (ed->col > 0) ed->col--;
            break;

        case DIR_RIGHT:
            if (ed->col < current_line(ed)->len) ed->col++;
            break;
    }
}

void ed_insert_char(Editor *ed, char c)
{
    String *line = current_line(ed);
    TODO("insert at col instead of append");
    str_append(line, (char)c);
    ed->col++;
}

void ed_insert_str(Editor *ed, const char *str)
{
    while (*str)
    {
        ed_insert_char(ed, *str);
        str++;
    }
}

void ed_newline(Editor *ed)
{
    TODO("insert line at row instead of append");
    alloc_line(ed);
    ed->row = ed->lines.len - 1;
    ed->col = 0;
}

void ed_backspace(Editor *ed)
{
    String *line = current_line(ed);
    if (line->len > 0)
    {
        TODO("delete at col instead of last");
        line->len--;
        str_append_null(line);
        ed->col--;
    }
    else
    {
        str_free(line);
        TODO("remove line at row instead of last");
        ed->lines.len--;
        ed->row--;
        ed->col = current_line(ed)->len - 1;
    }
}

size_t ed_line_count(Editor *ed)
{
    return ed->lines.len;
}

const char *ed_getline(Editor *ed, size_t row)
{
    return ed->lines.data[row]->data;
}
