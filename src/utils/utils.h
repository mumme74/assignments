
#ifndef _UTILS_H_
#define _UTILS_H_

#include <stddef.h>


#define STR_IMPL(str) #str
/// used to stringize a value
#define STR(str) STR_IMPL(str)

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
size_t printlen_utf8(const char *str, size_t maxlen);

#endif // _UTILS_H_
