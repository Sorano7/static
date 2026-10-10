#include "buffer.h"
#include "cut.h"
#include <ctype.h>

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

static void set_col(Buffer *buf, size_t col)
{
    buf->col = col;
    buf->target_col = col;
}

void buf_clear(Buffer *buf)
{
    set_col(buf, 0);
    buf->row = 0;
    DA_FOREACH(&buf->lines, String *, line)
        str_free(*line);
    buf->lines.len = 1;
}

size_t buf_line_count(const Buffer *buf)
{
    return buf->lines.len;
}

StringView buf_getline(const Buffer *buf, size_t row)
{
    return SV(buf->lines.data[row]);
}

static size_t wrap_line(StringView line, SVList *out, size_t *col, size_t max_cols)
{
    size_t count = 0;

    while (max_cols > 0 && line.len > max_cols)
    {
        size_t split = max_cols + 1;
        while (split > 0 && !isspace(line.data[split - 1]))
            split--;
        if (split == 0 && !isspace(line.data[0]))
            split = max_cols;

        StringView next = sv_shift(&line, split);

        da_append(out, next);
        if (!col)
        {
            count++;
            continue;
        }

        if (*col > next.len)
        {
            *col -= next.len;
            count++;
        }
    }

    da_append(out, line);
    return ++count;
}

static void buf_all_lines(const Buffer *buf, SVList *out, size_t *out_row, size_t *out_col, size_t max_cols)
{
    size_t row = buf->row;
    size_t col = buf->col;

    size_t real_row = row;

    for (size_t i = 0; i < real_row; i++)
    {
        size_t row_count = wrap_line(buf_getline(buf, i), out, nullptr, max_cols);
        row += row_count - 1;
    }

    size_t row_count = wrap_line(buf_getline(buf, real_row), out, &col, max_cols);
    if (row_count > 0) row += row_count - 1;

    for (size_t i = real_row+1; i < buf->lines.len; i++)
        wrap_line(buf_getline(buf, i), out, nullptr, max_cols);

    *out_row = row;
    *out_col = col;
}

BufferView *buf_view(const Buffer *buf, size_t max_cols)
{
    BufferView *view = calloc(1, sizeof(BufferView));
    da_init(&view->lines);
    buf_all_lines(buf, &view->lines, &view->row, &view->col, max_cols);
    return view;
}

void buf_view_free(BufferView *view)
{
    if (!view) return;
    da_free(&view->lines);
    free(view);
}

static String *get_current_line(Buffer *buf)
{
    return buf->lines.data[buf->row];
}

static void clamp_col(Buffer *buf)
{
    size_t line_len = get_current_line(buf)->len;
    if (buf->target_col > line_len)
    {
        buf->col = line_len;
    }
    else
    {
        if (buf->col < buf->target_col)
            buf->col = buf->target_col;
    }
}

static void clamp_row(Buffer *buf)
{
    if (buf->row > buf->lines.len - 1)
        buf->row = buf->lines.len - 1;
}

static void clamp_target_col(Buffer *buf)
{
    size_t line_len = get_current_line(buf)->len;
    if (buf->target_col > line_len)
        buf->target_col = line_len;
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
            clamp_target_col(buf);
            if (buf->target_col > 0)
                buf->target_col--;

            buf->col = buf->target_col;
            clamp_col(buf);
            break;

        case DIR_RIGHT:
            clamp_target_col(buf);
            if (buf->target_col < get_current_line(buf)->len)
                buf->target_col++;
            buf->col = buf->target_col;
            clamp_col(buf);
            break;
    }
}

void buf_move_cursor(Buffer *buf, Direction dir, bool by_word)
{
    bool left = dir == DIR_LEFT;
    bool right = dir == DIR_RIGHT;
    if ((left || right) && by_word)
    {
        String *line = get_current_line(buf);

        bool space_start = isspace(line->data[buf->col]);
        move_cursor(buf, dir);

        while (buf->col > 0 && buf->col < line->len)
        {
            if (space_start != isspace(line->data[buf->col]))
                break;

            move_cursor(buf, dir);
        }
    }
    else
    {
        move_cursor(buf, dir);
    }
}

void buf_insert_chr(Buffer *buf, char c)
{
    String *line = get_current_line(buf);
    str_insert_char(line, (char)c, buf->col);
    buf->col++;
    buf->target_col++;
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
    set_col(buf, 0);
}

void buf_insert_line(Buffer *buf, Direction dir)
{
    DEV_MUST(dir == DIR_UP || dir == DIR_DOWN);
    bool below = dir == DIR_DOWN;
    alloc_newline(buf, below ? ++buf->row : buf->row);
    set_col(buf, 0);
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
        set_col(buf, 0);
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
        set_col(buf, buf->col-1);
        return;
    }

    if (buf->lines.len > 1 && buf->row > 0)
    {
        String *prev = buf->lines.data[buf->row-1];
        set_col(buf, prev->len);

        if (line->len > 0)
            str_append(prev, line);

        free_current_line(buf);
        buf->row--;
    }
}

void buf_delete_word(Buffer *buf)
{
    if (buf->col == 0)
    {
        buf_delete_chr(buf);
        return;
    }

    String *line = get_current_line(buf);
    bool space_start = isspace(line->data[buf->col-1]);
    while (buf->col > 0)
    {
        bool space = isspace(line->data[buf->col-1]);
        if (space_start != space)
            break;
        buf_delete_chr(buf);
    }
}

void buf_load_string(Buffer *buf, String *str)
{
    StringView sv = SV(str);
    while (sv.len > 0)
    {
        StringView line = sv_split(&sv, '\n');
        SV_TO_CSTR(line, s);
        buf_insert_str(buf, s);
        if (sv.len > 0) buf_split_line(buf);
    }

    buf->row = 0;
    set_col(buf, 0);
}

void buf_to_string(const Buffer *buf, String *out)
{
    DA_FOR(&buf->lines, i)
    {
        String *line = buf->lines.data[i];
        str_append(out, line);
        if (i < buf->lines.len-1)
            str_append(out, "\n");
    }
}
