#include <assert.h>
#include <string.h>
#include "arena.h"

#include "typestring.h"


#define XOR_VLU 0xA5

// -------------------------------------------------

void types_string_init(types_String *str)
{
    str->len = str->size = 0;
    str->data = NULL;
}

bool types_string_pre_alloc(
    types_String *str, uint32_t size, mem_Arena *arena)
{
    types_string_init(str);
    str->data = mem_arena_alloc(arena, size+1);
    if (!str->data)
        return false;

    str->size = size+1;
    memset(str->data, 0, size+1);

    return true;
}

bool types_string_push_str(
    types_String *dest, const char *src,
    uint32_t sz, mem_Arena *arena
) {
    if (dest->size - dest->len < sz+1) {
        char *old = dest->data;
        uint32_t old_len = dest->len;
        if (!types_string_pre_alloc(dest, sz + old_len +1, arena))
            return false;

        memcpy(dest->data, old, old_len);
        dest->len = old_len;
    }

    memcpy(&dest->data[dest->len], src, sz);
    dest->len += sz;
    dest->data[dest->len] = '\0';

    return true;
}

void types_string_scramble(types_String *dest, types_String *src, uint32_t scramble)
{
    assert(dest->size >= src->len);

    for (uint32_t i = 0; i < src->len; ++i) {
        uint8_t shift = (i % 4) * 8;
        uint8_t sc = (scramble & (0xff << shift)) >> shift;
        uint8_t b = (src->data[i] + sc) ^ XOR_VLU;
        dest->data[i] = b;
    }
}

void types_string_unscramble(types_String *dest, types_String *src, uint32_t scramble)
{
    assert(dest->size >= src->len);

    for (uint32_t i = 0; i < src->len; ++i) {
        uint8_t shift = (i % 4) * 8;
        uint8_t sc = (scramble & (0xff << shift)) >> shift;
        uint8_t b = (src->data[i] ^ XOR_VLU) - sc;
        dest->data[i] = b;
    }
}