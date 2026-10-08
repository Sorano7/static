#include "editor.h"
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct Editor
{
    LineList lines;
    size_t row, col;
} Editor;

static void alloc_newline(Editor *ed, size_t at)
{
    String *line = malloc(sizeof(String));
    str_init(line);
    da_insert(&ed->lines, line, at);
}

Editor *editor_create(void)
{
    Editor *ed = calloc(1, sizeof(Editor));
    da_init(&ed->lines);
    alloc_newline(ed, 0);
    return ed;
}

void editor_free(Editor *ed)
{
    if (!ed) return;
    DA_FOREACH(&ed->lines, String *, line)
        str_free(*line);
    da_free(&ed->lines);
}

size_t editor_row(Editor *ed)
{
    return ed->row;
}

size_t editor_col(Editor *ed)
{
    return ed->col;
}

size_t editor_line_count(Editor *ed)
{
    return ed->lines.len;
}

const char *editor_getline(Editor *ed, size_t row)
{
    return ed->lines.data[row]->data;
}

static String *get_current_line(Editor *ed)
{
    return ed->lines.data[ed->row];
}

static void clamp_col(Editor *ed)
{
    size_t line_len = get_current_line(ed)->len;
    if (ed->col > line_len) ed->col = line_len;
}

static void clamp_row(Editor *ed)
{
    if (ed->row > ed->lines.len - 1)
        ed->row = ed->lines.len - 1;
}

static void move_cursor(Editor *ed, Direction dir)
{
    switch (dir)
    {
        case DIR_NONE:
            break;

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
            if (ed->col < get_current_line(ed)->len) ed->col++;
            break;
    }
}

static void insert_chr(Editor *ed, char c)
{
    String *line = get_current_line(ed);
    str_insert_char(line, (char)c, ed->col);
    ed->col++;
}

static void insert_str(Editor *ed, const char *str)
{
    while (*str)
    {
        insert_chr(ed, *str);
        str++;
    }
}

static void handle_split_line(Editor *ed)
{
    if (ed->col == 0)
    {
        alloc_newline(ed, ed->row++);
    }
    else
    {
        String *line = get_current_line(ed);
        alloc_newline(ed, ++ed->row);

        if (ed->col != line->len)
        {
            StringView after = SV(line);
            StringView before = sv_shift(&after, ed->col);
            str_append(get_current_line(ed), after);
            line->len = before.len;
            str_append_null(line);
        }
    }
    ed->col = 0;
}

static void handle_insert_line(Editor *ed, bool below)
{
    alloc_newline(ed, below ? ++ed->row : ed->row);
    ed->col = 0;
}

static void free_current_line(Editor *ed)
{
    str_free(get_current_line(ed));
    da_remove(&ed->lines, ed->row);
}

static void handle_remove_line(Editor *ed)
{
    if (ed->lines.len == 1)
    {
        str_reset(get_current_line(ed));
        ed->col = 0;
        return;
    }

    free_current_line(ed);
    clamp_row(ed);
    clamp_col(ed);
}

static void handle_delete(Editor *ed)
{
    String *line = get_current_line(ed);
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

        free_current_line(ed);
        ed->row--;
    }
}

void editor_handle_action(Editor *ed, Action action)
{
    switch (action.kind)
    {
        case ACTION_INSERT_CHR:    insert_chr(ed, action.chr);    break;
        case ACTION_INSERT_STR:    insert_str(ed, action.str);    break;
        case ACTION_DELETE:        handle_delete(ed);             break;
        case ACTION_REMOVE_LINE:   handle_remove_line(ed);        break;
        case ACTION_NEWLINE_BELOW: handle_insert_line(ed, true);  break;
        case ACTION_NEWLINE_ABOVE: handle_insert_line(ed, false); break;
        case ACTION_SPLIT_LINE:    handle_split_line(ed);         break;
        case ACTION_CURSOR_MOVE:   move_cursor(ed, action.dir);   break;
        default:                   UNREACHABLE();
    }
}
