#include "platform.h"

bool save_to_file(const Buffer *buf, StringView path)
{
    SV_TO_CSTR(path, path_buf);
    FILE *f = fopen(path_buf, "w");
    if (!f) return false;

    String sb;
    str_init(&sb);
    buf_to_string(buf, &sb);

    fwrite(sb.data, sb.len, 1, f);
    fclose(f);
    str_free(&sb);
    return true;
}

bool load_from_file(Buffer *buf, StringView path)
{
    SV_TO_CSTR(path, path_buf);
    FILE *f = fopen(path_buf, "r");
    if (!f) return false;

    String sb;
    str_init(&sb);

    bool ok = str_readfile(&sb, f);
    fclose(f);

    if (!ok)
    {
        str_free(&sb);
        return false;
    }

    buf_load_string(buf, &sb);
    str_free(&sb);
    return true;
}
