#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "arena.h"

/**
 * Stores a string in a scrambled form
 */
typedef struct {
    uint32_t len; ///< how many bytes
    uint32_t size; ///< allocated size
    char *data; ///< Holds len amounts of str data, null terminated
} types_String;

/**
 * Initialize a string
 */
void types_string_init(types_String *str);

/**
 * Initialize a string to to size using arena
 *
 * @param str The string to alloc to
 * @param size The wanted size
 * @param arena Allocate using arena
 * @return false if failed
 */
bool types_string_pre_alloc(types_String *str, uint32_t size, mem_Arena *arena);

/**
 * Append a C string to end
 *
 * @param dest The string to grow
 * @param src The C string to append
 * @param sz The size of src
 * @param arena The arena to use when needing to grow.
 * @return false if failed
 */
bool types_string_push_str(
    types_String *dest, const char *src,
    uint32_t sz, mem_Arena *arena);

/**
 * Scramble a string using scramble
 *
 * @param dest The scrambled string arrives here
 * @param src The string to scramble
 * @param scramble Using this scramble key
 */
void types_string_scramble(types_String *dest, types_String *src, uint32_t scramble);

/**
 * Unscramble a string using scramble
 *
 * @param dest The unscrambled string arrives here
 * @param src The scrambled string to unscramble
 * @param scramble Using this scramble key
 */
void types_string_unscramble(types_String *dest, types_String *src, uint32_t scramble);

#endif // _TYPES_H_
