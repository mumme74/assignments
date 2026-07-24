#include <assert.h>
#include <string.h>
#include "arena.h"


// create a char array used as a string
#define NAME String
#define T char
#define ARRAY_IMPLEMENTATION
#include "array.template.h"

// create an Array Strings
#define NAME StringArr
#define T String
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) strcmp(a.elements, b.elements) == 0
#include "array.template.h"

#define NO_TEMPLATE_STRING
#include "typestring.h"


#define XOR_VLU 0xA5

// -------------------------------------------------

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
