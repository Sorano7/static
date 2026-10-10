#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include "cut.h"

DA_DEFINE(LineList, String *);

typedef struct
{
    LineList lines;
    size_t target_col;
    size_t row, col;
} Buffer;

typedef struct
{
    SVList lines;
    size_t row, col;
} BufferView;

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

// Create a view of the buffer with optional wrapping.
BufferView *buf_view(const Buffer *buf, size_t max_cols);

// Free a buffer view.
void buf_view_free(BufferView *view);

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
