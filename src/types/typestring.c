#include <assert.h>
#include <string.h>
#include "arena.h"
#include "utils.h"


// create a char array used as a string
#define NAME String
#define T char
#define ARRAY_IMPLEMENTATION
#include "array.template.h"

// create an Array Strings
#define NAME StringArr
#define T String
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) \
    a.size == b.size && \
    strncmp(a.elements, b.elements, a.size) == 0
#include "array.template.h"

#define NO_TEMPLATE_STRING
#include "typestring.h"


#define XOR_VLU 0xA5

// -------------------------------------------------

bool String_set(String* string, const char* text, uint32_t sz)
{
    String_clear(string);
    return String_append_str(string, text, sz);
}

bool String_set_string(String* dest, String* src)
{
    String_clear(dest);
    return String_append_str(dest, src->elements, src->size);
}

bool String_append_str(
    String *dest, const char *src, uint32_t sz
) {
    size_t needed = dest->size + sz+1;
    if (dest->capacity <= needed &&
        !String_resize(dest, needed)
    )
        return false;

    memcpy(&dest->elements[dest->size], src, sz);
    dest->size += sz;
    dest->elements[dest->size] = '\0';

    return true;
}

bool String_append_string(String* dest, String* src)
{
    return String_append_str(dest, src->elements, src->size);
}

bool String_append_from_slice(String* dest, StringSlice* source)
{
    return String_append_str(dest, source->elements, source->size);
}

bool String_set_from_slice(String* dest, StringSlice* source)
{
    String_clear(dest);
    return String_append_from_slice(dest, source);
}


bool String_pad(String* string, size_t length, const char ch)
{
    size_t needed = string->size + length+1;
    if (string->capacity <= needed &&
        !String_resize(string, needed)
    )
        return false;

    memset(&string->elements[string->size], ch, length);
    string->size += length;

    return true;
}

StringArr *String_split(String* string, const char* key, mem_Arena* arena)
{
    assert(key != NULL);
    assert(arena != NULL);

    StringArr *arr = (StringArr*)mem_arena_alloc(arena, sizeof(StringArr));
    if (!arr) return NULL;
    StringArr_init(arr, arena);

    const size_t keylen = MIN(strnlen(key, 10), 10);

    size_t linepos = 0, i = 0;
    for (; i < string->size; ++i) {
        if (string->elements[i] == key[0] &&
            strncmp(&string->elements[i], key, keylen) == 0
        ) {
            String *row = (String*)mem_arena_alloc(arena, sizeof(String));
            String_init(row, arena);
            String_set(row, &string->elements[linepos], i - linepos);
            StringArr_push_back(arr, *row);
            linepos = i + keylen;
        }
    }

    // handle the last row
    String *row = (String*)mem_arena_alloc(arena, sizeof(String));
    String_init(row, arena);
    String_set(row, &string->elements[linepos], i - linepos);
    StringArr_push_back(arr, *row);

    return arr;
}

size_t String_utf8_len(String* string)
{
    if (!string) return 0;

    size_t len = 0, maxlen = string->size;
    for (const char *p = string->elements; *p != '\0' && maxlen--; ++p) {
        if ((*p & 0xC0) != 0x80)
            ++len;
    }

    return len;
}

StringSlice String_uft8_slice(String* string, size_t start, uint32_t length)
{
    const char *st_pos = utf8_pos_length_in(
        string->elements, string->size, start);

    size_t pos_in_str = st_pos - string->elements;

    const char *end_pos = utf8_pos_length_in(
        st_pos, string->size - pos_in_str, length);
    size_t slice_byte_len = end_pos - st_pos;

    return String_slice(string, pos_in_str, slice_byte_len);
}

bool String_scramble(String *dest, String *src, uint32_t scramble)
{
    if (dest->capacity < src->size &&
        !String_resize(dest, src->size)
    )
        return false;

    for (uint32_t i = 0; i < src->size; ++i) {
        uint8_t shift = (i % 4) * 8;
        uint8_t sc = (scramble & (0xff << shift)) >> shift;
        uint8_t b = (src->elements[i] + sc) ^ XOR_VLU;
        dest->elements[i] = b;
    }

    return true;
}

bool String_unscramble(String *dest, String *src, uint32_t scramble)
{
    if (dest->capacity < src->size &&
        !String_resize(dest, src->size)
    )
        return false;

    for (uint32_t i = 0; i < src->size; ++i) {
        uint8_t shift = (i % 4) * 8;
        uint8_t sc = (scramble & (0xff << shift)) >> shift;
        uint8_t b = (src->elements[i] ^ XOR_VLU) - sc;
        dest->elements[i] = b;
    }

    return true;
}

// --------------------------------------------------------------

String* StringArr_join(StringArr* arr, const char *sep, mem_Arena* arena)
{
    assert(arena != NULL);

    String *str = (String*)mem_arena_alloc(arena, sizeof(String));
    if (!str) return NULL;
    String_init(str, arena);

    if (arr->size == 0)
        return str;

    const size_t MAX_SEP = 10;

    String_set(str, arr->elements[0].elements, arr->elements[0].size);

    for (size_t i = 1; i < arr->size; ++i) {
        if (sep)
            String_append_str(str, sep, strnlen(sep, MAX_SEP));

        String_append_str(str, arr->elements[i].elements, arr->elements[i].size);
    }

    return str;
}



bool StringArr_append(StringArr* arr, const char* str, int length)
{
    String tmp;
    String_init(&tmp, arr->arena);
    size_t size = length > -1 ? (size_t) length : strlen(str) - length;
    if (!String_set(&tmp, str, size))
        return false;

    return StringArr_push_back(arr, tmp);
}