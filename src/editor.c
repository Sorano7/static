#include "editor.h"
#include <math.h>
#include <ctype.h>

constexpr float padding           = 50;
constexpr float line_spacing      = 2;
constexpr float font_size_step    = 2;
constexpr float default_font_size = 28;
constexpr float max_input_rate    = 100;

void editor_default_opt(Editor *ed)
{
    ed->opt = (RenderOpt){
        .font        = LoadFontEx("res/IosevkaWide-Regular.ttf", 64, NULL, 0),
        .font_size   = default_font_size,
        .fg          = GetColor(0xd5dde3ff),
        .bg          = GetColor(0x181e29ff),
        .text_effect = true,
        .text_wrap   = true,
    };
}

void editor_init(Editor *ed, Buffer *buf)
{
    memset(ed, 0, sizeof(*ed));
    if (!buf) buf = buf_create();
    ed->buf = buf;
    ed->input_rate = 0;

    editor_default_opt(ed);
}

void editor_free(Editor *ed)
{
    if (!ed) return;
    if (ed->buf) buf_free(ed->buf);
}

void editor_set_opt(Editor *ed, const RenderOpt *opt)
{
    ed->opt = *opt;
}

#define key_pressed_or_held(key) (IsKeyPressed(key) || IsKeyPressedRepeat(key))

static void handle_input(Editor *ed)
{
    Buffer *buf = ed->buf;

    int key = GetCharPressed();
    while (key > 0)
    {
        if ((key >= 32) && (key <= 125))
            buf_insert_chr(buf, key);
        key = GetCharPressed();
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (key_pressed_or_held(KEY_BACKSPACE))
    {
        if (shift)
            buf_remove_line(buf);
        else
            buf_delete_chr(buf);
    }

    if (key_pressed_or_held(KEY_ENTER))
    {
        if (shift)
            buf_insert_line(buf, DIR_UP);
        else if (ctrl)
            buf_insert_line(buf, DIR_DOWN);
        else
            buf_split_line(buf);
    }

    if (IsKeyPressed(KEY_TAB)) buf_insert_str(buf, "    ");

    bool left  = key_pressed_or_held(KEY_LEFT)  || (ctrl && key_pressed_or_held(KEY_H));
    bool down  = key_pressed_or_held(KEY_DOWN)  || (ctrl && key_pressed_or_held(KEY_J));
    bool up    = key_pressed_or_held(KEY_UP)    || (ctrl && key_pressed_or_held(KEY_K));
    bool right = key_pressed_or_held(KEY_RIGHT) || (ctrl && key_pressed_or_held(KEY_L));

    if (left)  buf_move_cursor(buf, DIR_LEFT);
    if (down)  buf_move_cursor(buf, DIR_DOWN);
    if (up)    buf_move_cursor(buf, DIR_UP);
    if (right) buf_move_cursor(buf, DIR_RIGHT);

    // UI actions
    if (ctrl)
    {
        if (IsKeyPressed(KEY_L))
        {
            Color prev_fg = ed->opt.fg;
            ed->opt.fg = ed->opt.bg;
            ed->opt.bg = prev_fg;
        }

        if (IsKeyPressed(KEY_W))
            ed->opt.text_wrap = !ed->opt.text_wrap;

        #ifdef _DEV
            if (IsKeyPressed(KEY_P))
                ed->opt.text_effect = !ed->opt.text_effect;
        #endif

        if (key_pressed_or_held(KEY_EQUAL)) ed->opt.font_size += font_size_step;
        if (key_pressed_or_held(KEY_MINUS)) ed->opt.font_size -= font_size_step;
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

static Buffer *wrap_lines(Editor *ed, size_t max_cols)
{
    const Buffer *buf = ed->buf;
    Buffer *out = buf_create();

    size_t prev_row = buf->row;
    size_t cursor_row = prev_row;
    size_t cursor_col = buf->col;

    for (size_t i = 0; i < buf_line_count(buf); i++)
    {
        StringView line = buf_getline(buf, i);

        while (line.len > max_cols)
        {
            size_t split = max_cols;
            for (; split > 0; split--)
                if (isspace(line.data[split])) break;
            if (split < max_cols) split++;
            if (split == 0) split = max_cols;

            StringView next = sv_shift(&line, split);
            SV_TO_CSTR(next, s);
            buf_insert_str(out, s);
            buf_insert_line(out, DIR_DOWN);

            if (prev_row == i)
            {
                if (cursor_col > next.len) 
                {
                    cursor_row++;
                    cursor_col -= next.len;
                }
            }
            else
            {
                cursor_row++;
            }
        }

        SV_TO_CSTR(line, s);
        buf_insert_str(out, s);

        if (i < buf_line_count(buf)-1)
            buf_insert_line(out, DIR_DOWN);
    }

    out->row = cursor_row;
    out->col = cursor_col;

    return out;
}

void editor_render(Editor *ed)
{
    RenderOpt opt = ed->opt;

    Font font = opt.font;

    Vector2 block = MeasureTextEx(font, "A", opt.font_size, 0);
    float line_height = line_spacing + block.y;

    float usable_w = GetScreenWidth() - (padding * 2);
    float usable_h = GetScreenHeight() - (padding * 2);
    size_t max_cols = usable_w / block.x;
    size_t max_rows = usable_h / block.y;

    String sb;
    str_init(&sb);

    Buffer *buf = ed->buf;
    bool need_free = false;

    if (opt.text_wrap)
    {
        buf = wrap_lines(ed, max_cols);
        need_free = true;
    }

    size_t row = buf->row;
    size_t col = buf->col;

    Vector2 pos = {padding, padding};
    if (row > max_rows)
        pos.y -= (row - max_rows) * line_height;
    if (col > max_cols)
        pos.x -= (col - max_cols) * block.x;

    ClearBackground(opt.bg);

    for (size_t i = 0; i < buf_line_count(buf); i++)
    {
        StringView line = buf_getline(buf, i);
        if (opt.text_effect)
            process_line(&sb, line, 1 - (ed->input_rate / max_input_rate));
        else
            str_append(&sb, line);

        DrawTextEx(opt.font, sb.data, pos, opt.font_size, 0, opt.fg);

        str_reset(&sb);

        if (i == row)
        {
            Rectangle cursor = {
                .x      = pos.x + block.x * col,
                .y      = pos.y,
                .width  = 2,
                .height = block.y,
            };
            DrawRectangleRec(cursor, opt.fg);
        }

        pos.y += line_height;
    }

    str_free(&sb);
    if (need_free) buf_free(buf);
}
