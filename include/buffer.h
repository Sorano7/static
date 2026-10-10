#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <raylib.h>
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct Buffer
{
    LineList lines;
    size_t target_col;
    size_t row, col;
} Buffer;

// Allocate a new buffer.
Buffer *buf_create(void);

// Free a buffer.
void buf_free(Buffer *buf);

// Clear the content of a buffer.
void buf_clear(Buffer *buf);

// Total number of lines.
size_t buf_line_count(const Buffer *buf);

// Get the line at row.
StringView buf_getline(const Buffer *buf, size_t row);

// Get a list of all lines wrapped at max_cols.
// No wrapping if max_cols is 0.
// Returns the adjusted cursor position {col, row}.
Vector2 buf_all_lines(const Buffer *buf, SVList *out, size_t max_cols);

typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} Direction;

// Move the cursor once in a direction.
void buf_move_cursor(Buffer *buf, Direction dir, bool by_word);

// Insert a character at cursor.
void buf_insert_chr(Buffer *buf, char c);

// Insert a string at cursor.
void buf_insert_str(Buffer *buf, const char *str);

// Delete a character at cursor.
void buf_delete_chr(Buffer *buf);

// Delete a word before the cursor.
void buf_delete_word(Buffer *buf);

// Split the current line at cursor.
void buf_split_line(Buffer *buf);

// Insert a newline above or below and move the cursor there.
void buf_insert_line(Buffer *buf, Direction dir);

// Remove the current line.
void buf_remove_line(Buffer *buf);

// Load a string into the buffer.
void buf_load_string(Buffer *buf, String *str);

// Convert the buffer to a newline-separated string.
void buf_to_string(const Buffer *buf, String *out);

#endif
