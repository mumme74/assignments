#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "utils.h"

static char *errorbuffer = NULL,
            *warningbuffer = NULL;
static size_t error_size = 0,
              warning_size = 0;
static bool _catch_output = false;

static void cleanup()
{
    free(errorbuffer);
    free(warningbuffer);
}

static void ensure_size(char** buf, size_t *sz)
{
    static const size_t threshold = 1024;
    if (*sz < threshold) {
        if (*sz == 0)
            atexit(cleanup);

        *sz += threshold * 2;
        *buf = realloc(*buf, *sz);
    }
}


/**
 * Tests if str is a number.
 *
 * @param str The string to test.
 * @return 1 if true, 0 otherwise.
 */
bool isnumber(const char *str)
{
    for (const char *cp = str; *cp != '\0'; ++cp) {
        if (!isspace(*cp) && !isdigit(*cp))
            return false;
    }

    return true;
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
uint32_t printlen_utf8(const char *str, uint32_t maxlen)
{
    uint32_t len = 0;
    for (const char *p = str; *p != '\0' && --maxlen; ++p) {
        if ((*p & 0xC0) != 0x80)
            ++len;
    }

    return len;
}

const char* utf8_pos_length_in(const char* buf, size_t maxlen, size_t length)
{
    if (!buf) return NULL;

    size_t len = 0; maxlen++;
    const char *p = buf;
    for (; *p != '\0' && --maxlen; ++p) {
        if ((*p & 0xC0) != 0x80) {
            if (len++ == length)
                break;
        }
    }

    return p;
}

void catch_output(int catch)
{
    _catch_output = catch;
}

const char* read_warning()
{
    return warningbuffer;
}

const char* read_error()
{
    return errorbuffer;
}


void clear_warning()
{
    memset(warningbuffer, 0, warning_size);
    warning_size = 0;
}

void clear_error()
{
    memset(errorbuffer, 0, error_size);
    error_size = 0;
}

void write_error(const char* format, ...)
{

    va_list args;
    va_start(args, format);

    if (_catch_output) {
        ensure_size(&errorbuffer, &error_size);
        sprintf(errorbuffer, format, args);
    } else {
        fprintf(stderr, format, args);
        putc('\n', stderr);
    }

    va_end(args);

}

void write_warning(const char* format, ...)
{

    va_list args;
    va_start(args, format);

    if (_catch_output) {
        ensure_size(&warningbuffer, &warning_size);
        sprintf(warningbuffer, format, args);
    } else {
        fprintf(stdin, format, args);
        putc('\n', stdin);
    }

    va_end(args);
}

bool is_dir(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) == 0)
        return false;

    return S_ISDIR(buf.st_mode) != 0;
}

bool is_file(const char* path)
{
    struct stat buf;
    if (stat(path, &buf) == 0)
        return false;

    return S_ISDIR(buf.st_mode) == 0;
}

