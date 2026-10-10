#include "editor.h"
#include <math.h>
#include <ctype.h>
#include "platform.h"

constexpr float padding           = 50; // Padding of the main buffer area
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
        .text_wrap   = false,
    };
}

void editor_init(Editor *ed, Buffer *buf)
{
    memset(ed, 0, sizeof(*ed));

    if (!buf) buf = buf_create();
    ed->buf = buf;
    ed->prompt.buf = buf_create();
    ed->prompt.cb = nullptr;

    ed->input_rate = 0;
    ed->mode = MODE_EDIT;

    editor_default_opt(ed);
    str_init(&ed->msg);
}

void editor_free(Editor *ed)
{
    if (!ed) return;
    if (ed->buf) buf_free(ed->buf);
    str_free(&ed->msg);
}

void editor_set_opt(Editor *ed, const RenderOpt *opt)
{
    ed->opt = *opt;
}

// Clear the previous message and set a new message.
static void set_msg(Editor *ed, const char *fmt, ...)
{
    va_list args;
    va_start(args);

    str_reset(&ed->msg);
    str_appendvf(&ed->msg, fmt, args);

    va_end(args);
}

#define key_pressed_or_held(key) (IsKeyPressed(key) || IsKeyPressedRepeat(key))

#define ctrl (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
#define shift (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))

// Handles buffer input for either main buffer or prompt buffer.
static void handle_buffer_input(Editor *ed)
{
    bool edit = ed->mode == MODE_EDIT;
    bool prompt = ed->mode == MODE_PROMPT;

    Buffer *buf = prompt ? ed->prompt.buf : ed->buf;

    int key = GetCharPressed();
    while (key > 0)
    {
        if ((key >= 32) && (key <= 125))
            buf_insert_chr(buf, key);
        key = GetCharPressed();
    }

    if (key_pressed_or_held(KEY_BACKSPACE))
    {
        if (edit && shift)
            buf_remove_line(buf);
        else
            buf_delete_chr(buf);
    }

    if (key_pressed_or_held(KEY_ENTER))
    {
        if (prompt)
        {
            if (ed->prompt.cb)
            {
                StringView input = buf_getline(ed->prompt.buf, 0);
                ed->prompt.cb(ed, input, ed->prompt.ud);
                str_reset(&ed->msg);
            }
            ed->mode = MODE_EDIT;
        }
        else
        {
            if (shift)
                buf_insert_line(buf, DIR_UP);
            else if (ctrl)
                buf_insert_line(buf, DIR_DOWN);
            else
                buf_split_line(buf);
        }
    }

    if (edit && IsKeyPressed(KEY_TAB)) buf_insert_str(buf, "    ");

    bool left  = key_pressed_or_held(KEY_LEFT)  || (ctrl && key_pressed_or_held(KEY_H));
    bool down  = key_pressed_or_held(KEY_DOWN)  || (ctrl && key_pressed_or_held(KEY_J));
    bool up    = key_pressed_or_held(KEY_UP)    || (ctrl && key_pressed_or_held(KEY_K));
    bool right = key_pressed_or_held(KEY_RIGHT) || (ctrl && key_pressed_or_held(KEY_L));

    if (left)  buf_move_cursor(buf, DIR_LEFT);
    if (right) buf_move_cursor(buf, DIR_RIGHT);

    if (edit)
    {
        if (down)  buf_move_cursor(buf, DIR_DOWN);
        if (up)    buf_move_cursor(buf, DIR_UP);
    }
}

// Set a prompt for input.
static void set_prompt(Editor *ed, StringView prompt, PromptCallBack cb, void *ud)
{
    ed->mode = MODE_PROMPT;
    buf_clear(ed->prompt.buf);
    ed->prompt.cb = cb;
    ed->prompt.ud = ud;

    str_reset(&ed->msg);
    str_append(&ed->msg, prompt);
}

static void on_save(Editor *ed, StringView path, void *ud)
{
    (void)ud;
    if (path.len == 0) return;

    bool ok = save_to_file(ed->buf, path);
    set_msg(ed, "%s: "SV_FMT, ok ? "Saved" : "Failed to save", SV_ARG(path));
}

static void on_load(Editor *ed, StringView path, void *ud)
{
    (void)ud;
    if (path.len == 0) return;

    buf_clear(ed->buf);
    bool ok = load_from_file(ed->buf, path);
    set_msg(ed, "%s: "SV_FMT, ok ? "Loaded" : "Failed to load", SV_ARG(path));
}

// Handle interface hotkeys.
static void handle_hotkey(Editor *ed)
{
    if (ctrl)
    {
        // Invert colors
        if (IsKeyPressed(KEY_D))
        {
            Color prev_fg = ed->opt.fg;
            ed->opt.fg = ed->opt.bg;
            ed->opt.bg = prev_fg;
        }

        // Set text wrap
        if (IsKeyPressed(KEY_W))
        {
            ed->opt.text_wrap = !ed->opt.text_wrap;
            set_msg(ed, "Text wrap: %s", ed->opt.text_wrap ? "on" : "off");
        }

        // Toggle text effect (dev only)
        #ifdef _DEV
            if (IsKeyPressed(KEY_P))
                ed->opt.text_effect = !ed->opt.text_effect;
        #endif

        // Font size
        bool equal = key_pressed_or_held(KEY_EQUAL);
        bool minus = key_pressed_or_held(KEY_MINUS);

        if (equal || minus)
        {
            ed->opt.font_size += equal ? font_size_step : -font_size_step;
            set_msg(ed, "Font size: %.0f", ed->opt.font_size);
        }

        if (IsKeyPressed(KEY_O))
        {
            set_prompt(ed, SV("Load: "), on_load, nullptr);
        }
        if (IsKeyPressed(KEY_S))
        {
            set_prompt(ed, SV("Save: "), on_save, nullptr);
        }
    }
}

// Increase the input rate by d (adjusted for frame time)
static void increase_input_rate(Editor *ed, float d)
{
    ed->input_rate += d * GetFrameTime();
    if (ed->input_rate > max_input_rate)
        ed->input_rate = max_input_rate;
}

static void update_input_rate(Editor *ed)
{
    // Adjust input rate
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

    // Decay input rate
    ed->input_rate *= expf(-GetFrameTime() * 1.0f);
    if (ed->input_rate < 0.5) ed->input_rate = 0;
}

void editor_update(Editor *ed)
{
    handle_buffer_input(ed);
    handle_hotkey(ed);
    update_input_rate(ed);
}

// Random int below n.
static int rand_below(int n)
{
    int limit = RAND_MAX - (RAND_MAX % n);
    int r;
    do { r = rand(); } while (r >= limit);
    return r % n;
}

// Random ascii character that's not c
static char rand_char(char c)
{
    int r = 32 + rand_below(94);
    if (r >= c) r++;
    return r;
}

// Text post-processing for a line.
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

// Render the status area.
static void render_status_area(Editor *ed)
{
    // Status area box
    Rectangle box = {
        .x      = 0,
        .y      = GetScreenHeight() - padding,
        .width  = GetScreenWidth(),
        .height = padding,
    };
    DrawRectangleRec(box, ed->opt.fg);

    float font_size = ed->opt.font_size * 0.8;
    Vector2 block = MeasureTextEx(ed->opt.font, "A", font_size, 0);
    Vector2 pos = {
        .x = padding / 2,
        .y = GetScreenHeight() - (padding/2) - (block.y/2),
    };

    if (ed->mode == MODE_EDIT)
    {
        DrawTextEx(ed->opt.font, ed->msg.data, pos, font_size, 0, ed->opt.bg);
    }
    else if (ed->mode == MODE_PROMPT)
    {
        String sb;
        str_init_with(&sb, SV(ed->msg));

        StringView prompt = buf_getline(ed->prompt.buf, 0);
        str_append(&sb, prompt);

        DrawTextEx(ed->opt.font, sb.data, pos, font_size, 0, ed->opt.bg);

        Rectangle cursor = {
            .x      = pos.x + block.x * (ed->prompt.buf->col + ed->msg.len),
            .y      = pos.y,
            .width  = 2,
            .height = block.y,
        };
        DrawRectangleRec(cursor, ed->opt.bg);
    }
}

// Render the main buffer.
static void render_buffer(Editor *ed)
{
    RenderOpt opt = ed->opt;

    Font font = opt.font;

    Vector2 block = MeasureTextEx(font, "A", opt.font_size, 0);
    float line_height = line_spacing + block.y;

    float usable_w = GetScreenWidth() - (padding * 2);
    float usable_h = GetScreenHeight() - (padding * 2);
    size_t max_cols = usable_w / block.x;
    size_t max_rows = usable_h / line_height;

    SVList lines;
    da_init(&lines);

    Vector2 cursor = buf_all_lines(ed->buf, &lines, opt.text_wrap ? max_cols : 0);
    size_t row = cursor.y, col = cursor.x;

    Vector2 pos = {padding, padding};

    if (row > max_rows)
        pos.y -= (row - max_rows) * line_height;
    if (col >= max_cols)
        pos.x -= (col - max_cols) * block.x;

    ClearBackground(opt.bg);

    String sb;
    str_init(&sb);

    DA_FOR(&lines, i)
    {
        StringView line = lines.data[i];
        if (opt.text_effect)
            process_line(&sb, line, 1-(ed->input_rate / max_input_rate));
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
    da_free(&lines);
}

void editor_render(Editor *ed)
{
    render_buffer(ed);
    render_status_area(ed);
}
