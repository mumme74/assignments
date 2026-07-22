#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>

#include "utils.h"

/**
 * Catches Ctrl+C and exit gracefully by cleaning up.
 *
 * Set up a signal catch in main to catch and close socket.
 *
 * @param sig The signal number that triggered it.
 */
void sigint_handler(int sig)
{
    close(get_socket());
    exit(0);
}

/**
 * Tests if str is a number.
 *
 * @param str The string to test.
 * @return 1 if true, 0 otherwise.
 */
int isnumber(const char *str)
{
    for (const char *cp = str; *cp != '\0'; ++cp) {
        if (!isspace(*cp) && !isdigit(*cp))
            return 0;
    }

    return 1;
}

/**
 * Finds the printable length of strings containing multibyte utf8 chars.
 *
 * @note This does not handle joining graphemes.
 *       Ie. 2 multibyte codepoints joining into one printable symbol.
 *       We would probably need a proper unicode lib for that.
 * @param str The string to check.
 * @param maxlen The maximum length of the string in ascii len.
 */
size_t printlen_utf8(const char *str, size_t maxlen)
{
    size_t len = 0;
    for (const char *p = str; *p != '\0' && --maxlen; ++p) {
        if ((*p & 0xC0) != 0x80)
            ++len;
    }

    return len;
}
