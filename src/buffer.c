#include "buffer.h"
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct Buffer
{
    LineList lines;
    size_t row, col;
} Buffer;

static void alloc_newline(Buffer *buf, size_t at)
{
    String *line = malloc(sizeof(String));
    str_init(line);
    da_insert(&buf->lines, line, at);
}

Buffer *buf_create(void)
{
    Buffer *buf = calloc(1, sizeof(Buffer));
    da_init(&buf->lines);
    alloc_newline(buf, 0);
    return buf;
}

void buf_free(Buffer *buf)
{
    if (!buf) return;
    DA_FOREACH(&buf->lines, String *, line)
        str_free(*line);
    da_free(&buf->lines);
}

size_t buf_row(Buffer *buf)
{
    return buf->row;
}

size_t buf_col(Buffer *buf)
{
    return buf->col;
}

size_t buf_line_count(Buffer *buf)
{
    return buf->lines.len;
}

StringView buf_getline(Buffer *buf, size_t row)
{
    return SV(buf->lines.data[row]);
}

static String *get_current_line(Buffer *buf)
{
    return buf->lines.data[buf->row];
}

static void clamp_col(Buffer *buf)
{
    size_t line_len = get_current_line(buf)->len;
    if (buf->col > line_len) buf->col = line_len;
}

static void clamp_row(Buffer *buf)
{
    if (buf->row > buf->lines.len - 1)
        buf->row = buf->lines.len - 1;
}

static void move_cursor(Buffer *buf, Direction dir)
{
    switch (dir)
    {
        case DIR_NONE:
            break;

        case DIR_UP:
            if (buf->row > 0) buf->row--;
            clamp_col(buf);
            break;

        case DIR_DOWN:
            buf->row++;
            clamp_row(buf);
            clamp_col(buf);
            break;

        case DIR_LEFT:
            if (buf->col > 0) buf->col--;
            break;

        case DIR_RIGHT:
            if (buf->col < get_current_line(buf)->len) buf->col++;
            break;
    }
}

static void insert_chr(Buffer *buf, char c)
{
    String *line = get_current_line(buf);
    str_insert_char(line, (char)c, buf->col);
    buf->col++;
}

static void insert_str(Buffer *buf, const char *str)
{
    while (*str)
    {
        insert_chr(buf, *str);
        str++;
    }
}

static void handle_split_line(Buffer *buf)
{
    if (buf->col == 0)
    {
        alloc_newline(buf, buf->row++);
    }
    else
    {
        String *line = get_current_line(buf);
        alloc_newline(buf, ++buf->row);

        if (buf->col != line->len)
        {
            StringView after = SV(line);
            StringView before = sv_shift(&after, buf->col);
            str_append(get_current_line(buf), after);
            line->len = before.len;
            str_append_null(line);
        }
    }
    buf->col = 0;
}

static void handle_insert_line(Buffer *buf, bool below)
{
    alloc_newline(buf, below ? ++buf->row : buf->row);
    buf->col = 0;
}

static void free_current_line(Buffer *buf)
{
    str_free(get_current_line(buf));
    da_remove(&buf->lines, buf->row);
}

static void handle_remove_line(Buffer *buf)
{
    if (buf->lines.len == 1)
    {
        str_reset(get_current_line(buf));
        buf->col = 0;
        return;
    }

    free_current_line(buf);
    clamp_row(buf);
    clamp_col(buf);
}

static void handle_delete(Buffer *buf)
{
    String *line = get_current_line(buf);
    if (buf->col > 0)
    {
        str_remove(line, buf->col-1);
        buf->col--;
        return;
    }

    if (buf->lines.len > 1 && buf->row > 0)
    {
        String *prev = buf->lines.data[buf->row-1];
        buf->col = prev->len;

        if (line->len > 0)
            str_append(prev, line);

        free_current_line(buf);
        buf->row--;
    }
}

void buf_handle_action(Buffer *buf, BufAction action)
{
    switch (action.kind)
    {
        case ACTION_INSERT_CHR:    insert_chr(buf, action.chr);    break;
        case ACTION_INSERT_STR:    insert_str(buf, action.str);    break;
        case ACTION_DELETE:        handle_delete(buf);             break;
        case ACTION_REMOVE_LINE:   handle_remove_line(buf);        break;
        case ACTION_NEWLINE_BELOW: handle_insert_line(buf, true);  break;
        case ACTION_NEWLINE_ABOVE: handle_insert_line(buf, false); break;
        case ACTION_SPLIT_LINE:    handle_split_line(buf);         break;
        case ACTION_CURSOR_MOVE:   move_cursor(buf, action.dir);   break;
        default:                   UNREACHABLE();
    }
}
