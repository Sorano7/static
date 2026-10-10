#ifndef PLATFORM_H
#define PLATFORM_H

#include "buffer.h"

// Save a buffer to file.
bool save_to_file(const Buffer *buf, StringView path);

// Load file content to buffer.
bool load_from_file(Buffer *buf, StringView path);

#endif
