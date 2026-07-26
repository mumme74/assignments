#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "arena.h"


#ifndef NO_TEMPLATE_STRING

// create a char array used as a string
#define NAME String
#define T char
//#define ARRAY_IMPLEMENTATION
#include "array.template.h"

#endif

/**
 * Set to text, clearin old content
 *
 * @param string The string to reset
 * @param text The text to insert
 * @param sz Length of text
 * @return false if failed
 */
bool String_set(String* string, const char* text, uint32_t sz);

/**
 * Append a C string to end
 *
 * @param dest The string to grow
 * @param src The C string to append
 * @param sz The size of src
 * @return false if failed
 */
bool String_append_str(String *dest, const char *src, uint32_t sz);

/**
 * Scramble a string using scramble
 *
 * @param dest The scrambled string arrives here
 * @param src The string to scramble
 * @param scramble Using this scramble key
 */
bool String_scramble(String *dest, String *src, uint32_t scramble);

/**
 * Unscramble a string using scramble
 *
 * @param dest The unscrambled string arrives here
 * @param src The scrambled string to unscramble
 * @param scramble Using this scramble key
 */
bool String_unscramble(String *dest, String *src, uint32_t scramble);

// -----------------------------------------------------------------------

#ifndef NO_TEMPLATE_STRING

// create an Array Strings
#define NAME StringArr
#define T String
//#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) strcmp(a.elements, b.elements) == 0
#include "array.template.h"

#endif

#endif // _TYPES_H_
