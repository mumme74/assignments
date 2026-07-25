#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "utils.h"

#include "terminal.h"

#define BUFFER_SIZE 8192


static struct termios orig_termios;

static char buffer[8192] = {0};
static char *wr_ptr = buffer;
static int cursor_x = 0,
           cursor_y = 0;
static int max_rows = 16,
           max_cols = 80;

static void update_win_size()
{
    // set max_rows, max_cols
    struct winsize win;
    ioctl(0, TIOCGWINSZ, &win);

    max_cols = win.ws_col;
    max_rows = win.ws_row;
}

// ------------------------------------------------------------

// VT100 code from: https://gist.github.com/viniciusdaniel/53a98cbb1d8cac1bb473da23f5708836

const struct _Format Format = {
    .Bold            = "1",
    .ResetBold       = "21",
    .Dim             = "2",
    .RestDim         = "22",
    .Underlined      = "4",
    .ResetUnderlined = "24",
    .Blink           = "5",
    .ResetBlink      = "25",
    .Invert          = "7",
    .ResetInvert     = "27",
    .Hidden          = "8",
    .ResetHidden     = "28",
    .ResetAll        = "0"
};


const struct _FrontColors FrontColors = {
    .Default      = "39",
    .Black        = "30",
    .Red          = "31",
    .Green        = "32",
    .Yellow       = "33",
    .Blue         = "34",
    .Magenta      = "35",
    .Cyan         = "36",
    .LightGray    = "37",
    .DarkGray     = "90",
    .LightRed     = "91",
    .LightGreen   = "92",
    .LightYellow  = "93",
    .LightBlue    = "94",
    .LightMAgenta = "95",
    .LightCyan    = "96",
    .White        = "97"
};

const struct _BackColors BackColors = {
    .Default      = "49",
    .Black        = "40",
    .Red          = "41",
    .Green        = "42",
    .Yellow       = "43",
    .Blue         = "44",
    .Magenta      = "45",
    .Cyan         = "46",
    .LightGray    = "47",
    .DarkGray     = "100",
    .LightRed     = "101",
    .LightGreen   = "102",
    .LightYellow  = "103",
    .LightBlue    = "104",
    .LightMagenta = "105",
    .LightCyan    = "106",
    .White        = "107"
};

// ---------------------------------------

void ui_one_command(const char* command)
{
    size_t sz = strnlen(command, 20);
    snprintf(wr_ptr, sz, "\x1b[%s", command);
    wr_ptr += sz;
}

void ui_many_commands(StringArr* arr)
{
    snprintf(wr_ptr, 3, "\x1b[");
    wr_ptr += 2;

    for (size_t i = 0; i < arr->size; ++i) {
        if (i > 0)
            snprintf(wr_ptr++, 2, ";");

        size_t sz = arr->elements[i].size;
        snprintf(wr_ptr, sz+1, "%s", arr->elements[i].elements);
        wr_ptr += sz;
    }

    snprintf(wr_ptr++, 2, "m");
}

void ui_printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    wr_ptr += vsnprintf(wr_ptr, BUFFER_SIZE, format, args);
    va_end(args);
}

void ui_get_cursor_pos(int* x, int *y)
{
    *x = cursor_x;
    *y = cursor_y;
}

void ui_get_screen_size(int* cols, int* rows)
{
    update_win_size();

    *cols = max_cols;
    *rows = max_rows;
}

void ui_set_cursor_pos(int x, int y)
{
    update_win_size();

    cursor_x = x > -1 ? MIN(x, max_cols) : 0;
    cursor_y = y > -1 ? MIN(y, max_rows) : 0;
    printf("\x1b[%d;%dH", cursor_y, cursor_x);
    fflush(stdout);
}

void ui_move_cursor_vert(int rows)
{
    update_win_size();

    if (rows < 0) { // up
        cursor_y = MAX(0, cursor_y + rows);
        printf("\x1b[%dA", cursor_y);
    } else if (rows > 0) { // down
        cursor_y = MIN(max_rows, cursor_y + rows);
        printf("\x1b[%dB", cursor_y);
    }
    fflush(stdout);
}

void ui_move_cursor_horz(int cols)
{
    update_win_size();

    if (cols < 0) { // left
        cursor_x = MAX(0, cursor_x + cols);
        printf("\x1b[%dD", cursor_x);
    } else if (cols > 0) { // right
        cursor_x = MIN(max_cols, cursor_x + cols);
        printf("\x1b[%dC", cursor_x);
    }
    fflush(stdout);
}

void ui_set_cursor_show(bool show)
{
    printf("%s", show ? "\x1b[?25h" : "\x1b[?25l");
    fflush(stdout);
}


void ui_clear_screen()
{
    update_win_size();

    // Clear screen
    printf("\x1b[H");
    for (int i = 0; i < max_rows; ++i) {
        for (int j = 0; j < max_cols; ++j) {
            putc(' ', stdout);
        }
        if (i < max_rows-1)
            putc('\n', stdout);
    }
    printf("\x1b[H");
    fflush(stdout);
}

void ui_render()
{
    ui_clear_screen();
    fwrite(buffer, BUFFER_SIZE, 1, stdout);
    wr_ptr = buffer;
    printf("\x1b[%d;%dH", cursor_y, cursor_x);
    fflush(stdout);
}

// Restore terminal to normal mode on exit
void ui_disable_raw_mode() {
    update_win_size();
    printf("\x1b[%d;%dH\n", max_rows, max_cols);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Enable raw mode: disable echo and buffering
void ui_enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(ui_disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Turn off echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main1() {
    ui_enable_raw_mode();
    int x = 5, y = 5;
    char c;

    while (1) {
        // 1. Clear screen and reset cursor
        printf("\x1b[2J\x1b[H");

        // 2. Draw UI boundaries
        printf("--- ANSI TUI (Press 'q' to quit) ---\n");
        for (int i = 0; i < 10; i++) {
            printf("|                                  |\n");
        }
        printf("------------------------------------\n");

        // 3. Move cursor and draw interactive element
        // \x1b[%d;%dH moves cursor to line Y, column X
        printf("\x1b[%d;%dH\x1b[1;32m@\x1b[0m", y, x);
        fflush(stdout);

        // 4. Read input
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'q') break;

            // Handle WASD movement keys
            if (c == 'w' && y > 2) y--;
            if (c == 's' && y < 11) y++;
            if (c == 'a' && x > 2) x--;
            if (c == 'd' && x < 35) x++;
        }
    }
    return 0;
}

#include "terminal.h"