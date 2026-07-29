#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "arena.h"

typedef struct StringArr StringArr;

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
 * Set (copy) from another string
 *
 * @param des, The string to set
 * @param src The string that has content
 * @return false if failed
 */
bool String_set_string(String* dest, String* dsrc);

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
 * Append a string from another string
 *
 * @param des, The string to append to
 * @param src The string that has content to append
 * @return false if failed
 */
bool String_append_string(String* dest, String* dsrc);

/**
 * Set string from a slice of another
 *
 * @param dest The string to set
 * @param source The slice used as source
 * @return false if failed
 */
bool String_set_from_slice(String* dest, StringSlice* source);

/**
 * Append string from a slice of another
 *
 * @param dest The string to append to
 * @param source The slice used as source
 * @return false if failed
 */
bool String_append_from_slice(String* dest, StringSlice* source);

/**
 * Pad a string with length chars of ch
 *
 * @param string The string to pad
 * @param length The length of the string
 * @param ch The char to pad with
 */
bool String_pad(String* string, size_t length, const char ch);

/**
 * Split a string into parts on the key
 *
 * @param string The string to split
 * @param key The key to split on, maxlen 10 chars, atleast 1
 * @param arena Use this arena to allocate
 * @return The parts in a StringArr
 */
StringArr *String_split(String* string, const char* key, mem_Arena* arena);

/**
 * Get the printable length (utf8 might be multibytes)
 *
 * @param string The string to count
 * @return the printable length as it would appear on screen
 */
size_t String_utf8_len(String* string);

/**
 * Get a slice of the utf8 printable length
 *
 * @param string The string to slice from
 * @param start Startpos
 * @param length End pos
 * @return Ths slice of string
 */
StringSlice String_uft8_slice(String* string, size_t start, uint32_t length);


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

/**
 * Append a single cStr to array
 * @param arr The String array to operate on
 * @param str The c str to add
 * @param length Th lenngth to insert, -1 is at end, -12 end-1 and so on.
 * @return false if failed
 */
bool StringArr_append(StringArr* arr, const char* str, int length);

/**
 * Join a StringArr into a single string with optional sep in between
 *
 * @param arr The StringArr to join
 * @param sep The separator in between, might be null, but no longer than 10 chars.
 * @param arena USe this arena to allocate
 * @return The joined string
 */
String* StringArr_join(StringArr* arr, const char *sep, mem_Arena* arena);

#endif // _TYPES_H_
