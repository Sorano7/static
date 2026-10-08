#include "editor.h"
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct Editor
{
    LineList lines;
    size_t row, col;
} Editor;

static void new_line(Editor *ed, size_t at)
{
    String *line = malloc(sizeof(String));
    str_init(line);
    da_insert(&ed->lines, line, at);
}

Editor *ed_create(void)
{
    Editor *ed = calloc(1, sizeof(Editor));
    da_init(&ed->lines);
    new_line(ed, 0);
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

static void clamp_col(Editor *ed)
{
    size_t line_len = current_line(ed)->len;
    if (ed->col > line_len) ed->col = line_len;
}

static void clamp_row(Editor *ed)
{
    if (ed->row > ed->lines.len - 1)
        ed->row = ed->lines.len - 1;
}

void ed_move(Editor *ed, Direction dir)
{
    switch (dir)
    {
        case DIR_UP:
            if (ed->row > 0) ed->row--;
            clamp_col(ed);
            break;

        case DIR_DOWN:
            ed->row++;
            clamp_row(ed);
            clamp_col(ed);
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
    str_insert_char(line, (char)c, ed->col);
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

void ed_linebreak(Editor *ed)
{
    if (ed->col == 0)
    {
        new_line(ed, ed->row++);
    }
    else
    {
        String *line = current_line(ed);
        new_line(ed, ++ed->row);

        if (ed->col != line->len)
        {
            StringView after = SV(line);
            StringView before = sv_shift(&after, ed->col);
            str_append(current_line(ed), after);
            line->len = before.len;
            str_append_null(line);
        }
    }
    ed->col = 0;
}

void ed_newline(Editor *ed, bool below)
{
    new_line(ed, below ? ++ed->row : ed->row);
    ed->col = 0;
}

static void remove_line(Editor *ed)
{
    str_free(current_line(ed));
    da_remove(&ed->lines, ed->row);
}

void ed_remove_line(Editor *ed)
{
    if (ed->lines.len == 1)
    {
        str_reset(current_line(ed));
        ed->col = 0;
        return;
    }

    remove_line(ed);
    clamp_row(ed);
    clamp_col(ed);
}

void ed_backspace(Editor *ed)
{
    String *line = current_line(ed);
    if (ed->col > 0)
    {
        str_remove(line, ed->col-1);
        ed->col--;
        return;
    }

    if (ed->lines.len > 1 && ed->row > 0)
    {
        String *prev = ed->lines.data[ed->row-1];
        ed->col = prev->len;

        if (line->len > 0)
            str_append(prev, line);

        remove_line(ed);
        ed->row--;
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
