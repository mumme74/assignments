
#ifndef _UTILS_H_
#define _UTILS_H_

#include <stddef.h>
#include <stdint.h>


#define STR_IMPL(str) #str
/// used to stringize a value
#define STR(str) STR_IMPL(str)

#define MIN(a,b) ((a)>(b) ? (b) : (a))
#define MAX(a,b) ((a)>(b) ? (a) : (b))


/**
 * Catches Ctrl+C and exit gracefully by cleaning up.
 *
 * Set up a signal catch in main to catch and close socket.
 *
 * @param sig The signal number that triggered it.
 */
void sigint_handler(int sig);

/**
 * Tests if str is a number.
 *
 * @param str The string to test.
 * @return 1 if true, 0 otherwise.
 */
int isnumber(const char *str);

/**
 * Finds the length of strings containing multibyte utf8 chars.
 *
 * @param str The string to check.
 * @param maxlen The maximum length of the string.
 */
uint32_t printlen_utf8(const char *str, uint32_t maxlen);

/**
 * Find the pos in string with len as printable len
 *
 * @param buf The string to search in
 * @param size The size of buf
 * @param utf8pos The sought pos
 * @returns the position in buf
 */
const char* utf8_pos_length_in(const char* buf, size_t size, size_t utf8pos);

/**
 * A single point for error messages
 */
void write_error(const char* errmsg);

/**
 * A single point to write warnings.
 */
void write_warning(const char* warning);

#endif // _UTILS_H_
