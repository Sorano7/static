#include "buffer.h"
#include "cut.h"

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
    free(buf);
}

size_t buf_line_count(const Buffer *buf)
{
    return buf->lines.len;
}

StringView buf_getline(const Buffer *buf, size_t row)
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

void buf_move_cursor(Buffer *buf, Direction dir)
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

void buf_insert_chr(Buffer *buf, char c)
{
    String *line = get_current_line(buf);
    str_insert_char(line, (char)c, buf->col);
    buf->col++;
}

void buf_insert_str(Buffer *buf, const char *str)
{
    while (*str)
    {
        buf_insert_chr(buf, *str);
        str++;
    }
}

void buf_split_line(Buffer *buf)
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

void buf_insert_line(Buffer *buf, Direction dir)
{
    DEV_MUST(dir == DIR_UP || dir == DIR_DOWN);
    bool below = dir == DIR_DOWN;
    alloc_newline(buf, below ? ++buf->row : buf->row);
    buf->col = 0;
}

static void free_current_line(Buffer *buf)
{
    str_free(get_current_line(buf));
    da_remove(&buf->lines, buf->row);
}

void buf_remove_line(Buffer *buf)
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

void buf_delete_chr(Buffer *buf)
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
