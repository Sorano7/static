#include "editor.h"
#include <math.h>
#include <ctype.h>

constexpr float padding           = 50;
constexpr float line_spacing      = 2;
constexpr float font_size_step    = 2;
constexpr float default_font_size = 28;
constexpr float max_input_rate    = 100;

void editor_init(Editor *ed, Buffer *buf)
{
    if (!buf) buf = buf_create();
    ed->buf = buf;
    ed->font_size = default_font_size;
    ed->input_rate = 0;
}

void editor_free(Editor *ed)
{
    if (!ed) return;
    if (ed->buf) buf_free(ed->buf);
}

#define key_pressed_or_held(key) (IsKeyPressed(key) || IsKeyPressedRepeat(key))

static void handle_input(Editor *ed)
{
    Buffer *buf = ed->buf;

    int key = GetCharPressed();
    while (key > 0)
    {
        if ((key >= 32) && (key <= 125))
        {
            buf_do(buf, ACTION_INSERT_CHR, .chr=(char)key);
        }
        key = GetCharPressed();
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (key_pressed_or_held(KEY_BACKSPACE))
    {
        if (shift)
            buf_do(buf, ACTION_REMOVE_LINE);
        else
            buf_do(buf, ACTION_DELETE);
    }

    if (key_pressed_or_held(KEY_ENTER))
    {
        if (shift)     buf_do(buf, ACTION_NEWLINE_ABOVE);
        else if (ctrl) buf_do(buf, ACTION_NEWLINE_BELOW);
        else           buf_do(buf, ACTION_SPLIT_LINE);
    }

    if (IsKeyPressed(KEY_TAB)) buf_do(buf, ACTION_INSERT_STR, .str="    ");

    if (key_pressed_or_held(KEY_LEFT))  buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_LEFT);
    if (key_pressed_or_held(KEY_DOWN))  buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_DOWN);
    if (key_pressed_or_held(KEY_UP))    buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_UP);
    if (key_pressed_or_held(KEY_RIGHT)) buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_RIGHT);

    if (ctrl)
    {
        if (key_pressed_or_held(KEY_H)) buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_LEFT);
        if (key_pressed_or_held(KEY_J)) buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_DOWN);
        if (key_pressed_or_held(KEY_K)) buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_UP);
        if (key_pressed_or_held(KEY_L)) buf_do(buf, ACTION_CURSOR_MOVE, .dir=DIR_RIGHT);

        if (key_pressed_or_held(KEY_EQUAL)) ed->font_size += font_size_step;
        if (key_pressed_or_held(KEY_MINUS)) ed->font_size -= font_size_step;
    }
}

static void increase_input_rate(Editor *ed, float d)
{
    ed->input_rate += d * GetFrameTime();
    if (ed->input_rate > max_input_rate)
        ed->input_rate = max_input_rate;
}

void editor_update(Editor *ed)
{
    handle_input(ed);

    int key;
    while ((key = GetKeyPressed()) > 0)
        increase_input_rate(ed, 500);

    for (int k = KEY_SPACE; k <= KEY_KB_MENU; k++)
    {
        if (!IsKeyDown(k) && !IsKeyPressedRepeat(k))
            continue;

        switch (k)
        {
            case KEY_LEFT_CONTROL:
            case KEY_RIGHT_CONTROL:
            case KEY_LEFT_ALT:
            case KEY_RIGHT_ALT:
            case KEY_LEFT_SHIFT:
            case KEY_RIGHT_SHIFT:
                break;

            default:
                increase_input_rate(ed, 300);
                break;
        }
    }

    ed->input_rate *= expf(-GetFrameTime() * 2.0f);
    if (ed->input_rate < 0.5) ed->input_rate = 0;
}

static int rand_below(int n)
{
    int limit = RAND_MAX - (RAND_MAX % n);
    int r;
    do { r = rand(); } while (r >= limit);
    return r % n;
}

static char rand_char(char c)
{
    int r = 32 + rand_below(94);
    if (r >= c) r++;
    return r;
}

static void process_line(String *out, StringView line, float ratio)
{
    size_t k = llround(ratio * line.len);
    str_append(out, line);

    for (size_t i = 0; i < line.len; i++)
    {
        if ((size_t)rand_below((line.len-i)) < k)
        {
            if (!isspace(line.data[i]))
            {
                out->data[i] = rand_char(line.data[i]);
                k--;
            }
        }
    }
}

typedef struct
{
    Vector2 pos;
    float line_height;
    size_t col;
    size_t max_len;
    RenderOpt *opt;
} LineRenderCtx;

typedef struct
{
    Vector2 last_line;
    Vector2 next_line;
    size_t col_offset;
} LineRenderResult;

static LineRenderResult render_line(Editor *ed, StringView line, LineRenderCtx *ctx)
{
    RenderOpt *opt = ctx->opt;
    Vector2 pos = ctx->pos;

    SV_TO_CSTR(line, line_buf);
    DrawTextEx(opt->font, line_buf, pos, ed->font_size, 0, opt->fg);

    size_t col = ctx->col;
    if (line.len < col) col -= line.len;

    return (LineRenderResult){
        .last_line = pos,
        .next_line = (Vector2){pos.x, pos.y + ctx->line_height},
        .col_offset = col,
    };
}

static LineRenderResult render_line_wrapped(Editor *ed, StringView line, LineRenderCtx *ctx)
{
    if (line.len <= ctx->max_len)
        return render_line(ed, line, ctx);

    StringView remaining = line;
    float max_len = ctx->max_len;

    while (remaining.len > max_len)
    {
        size_t split = max_len;
        for (; split > 0; split--)
            if (isspace(remaining.data[split])) break;

        if (split < max_len) split++;
        if (split == 0) split = max_len;

        StringView next = sv_shift(&remaining, split);
        LineRenderResult res = render_line(ed, next, ctx);
        ctx->pos = res.next_line;
        ctx->col = res.col_offset;
    }

    return render_line(ed, remaining, ctx);
}

void editor_render(Editor *ed, RenderOpt *opt)
{
    Font font = opt->font;

    Vector2 block = MeasureTextEx(font, "A", ed->font_size, 0);
    float line_h = line_spacing + block.y;

    ClearBackground(opt->bg);

    String sb;
    str_init(&sb);

    float usable_w = GetScreenWidth() - (padding * 2);
    size_t max_len = usable_w / block.x;

    LineRenderCtx ctx = {
        .line_height = line_h,
        .max_len     = max_len,
        .opt         = opt,
        .pos         = {padding, padding},
        .col         = 0,
    };

    for (size_t i = 0; i < buf_line_count(ed->buf); i++)
    {
        size_t row = buf_row(ed->buf);
        size_t col = buf_col(ed->buf);
        ctx.col = col;

        StringView line = buf_getline(ed->buf, i);
        if (opt->text_effect)
            process_line(&sb, line, 1 - (ed->input_rate / max_input_rate));
        else
            str_append(&sb, line);

        LineRenderResult res = render_line_wrapped(ed, SV(sb), &ctx);
        str_reset(&sb);

        if (i == row)
        {
            Rectangle cursor = {
                .x      = res.last_line.x + block.x * res.col_offset,
                .y      = res.last_line.y,
                .width  = 2,
                .height = block.y,
            };
            DrawRectangleRec(cursor, opt->fg);
        }

        ctx.pos = res.next_line;
    }

    str_free(&sb);
}
