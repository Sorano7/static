#include "editor.h"
#include <math.h>

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
            out->data[i] = rand_char(line.data[i]);
            k--;
        }
    }
}

void editor_render(Editor *ed, RenderOpt *opt)
{
    Font font = opt->font;

    Vector2 line_size = MeasureTextEx(font, "A", ed->font_size, 0);
    float line_h = line_spacing + line_size.y;

    ClearBackground(GetColor(0x181e29ff));

    Color fg = RAYWHITE;
    float rate_ratio = 1 - (ed->input_rate / max_input_rate);
    // fg.a = rate_ratio > 1.0f ? 0xff : 0xff * rate_ratio;

    String sb;
    str_init(&sb);

    for (size_t i = 0; i < buf_line_count(ed->buf); i++)
    {
        StringView line = buf_getline(ed->buf, i);
        process_line(&sb, line, rate_ratio);

        Vector2 pos = {padding, i * line_h + padding};
        DrawTextEx(font, sb.data, pos, ed->font_size, 0, fg);
        str_reset(&sb);

        size_t row = buf_row(ed->buf);
        size_t col = buf_col(ed->buf);

        if (i == row)
        {
            Rectangle cursor = {
                .x      = padding + line_size.x * col,
                .y      = row * line_h + padding,
                .width  = line_size.x,
                .height = line_size.y,
            };
            DrawRectangleRec(cursor, fg);

            if (col < line.len)
            {
                SV_TO_CSTR(sv_slice(line, .from=col, .to=col+1), cursor_char);
                pos.x += line_size.x * col;
                DrawTextEx(font, cursor_char, pos, ed->font_size, 0, BLACK);
            }
        }
    }

    str_free(&sb);
}
