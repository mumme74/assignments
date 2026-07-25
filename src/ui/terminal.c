#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "utils.h"

#include "terminal.h"

#define BUFFER_SIZE 8192
#define ESC 27


static struct termios orig_termios;

static char buffer[8192] = {0};
static char *wr_ptr = buffer;
static int cursor_x = 1,
           cursor_y = 1,
           frm_x = 1,
           frm_y = 1;
static int max_rows = 16,
           max_cols = 80;

static bool show_cursor = true;

static void update_win_size()
{
    // set max_rows, max_cols
    struct winsize win;
    ioctl(0, TIOCGWINSZ, &win);

    max_cols = win.ws_col;
    max_rows = win.ws_row;
}

static void frm_top_left()
{
    wr_ptr += sprintf(wr_ptr, "\x1b[H");
}

static void frm_hide_cursor(bool hide)
{
    wr_ptr += sprintf(wr_ptr, hide ? "\x1b[?25l" : "\x1b[?25h");
}

static void frm_reset_buffer()
{
    memset(buffer, 0, BUFFER_SIZE);
    wr_ptr = buffer;
}

static int esc_bracket1(int c, char seq[])
{
    switch (seq[2]) {
    case '5': return Key_F5;
    case '7': return Key_F6;
    case '8': return Key_F7;
    case '9': return Key_F8;
    default: return c;
    }
}

static int esc_bracket2(int c, char seq[])
{
    switch (seq[2]) {
    case '0': return Key_F9;
    case '1': return Key_F10;
    case '3': return Key_F11;
    case '4': return Key_F12;
    case '~': return Key_Insert;
    default: return c;
    }
}

static int esc_bracket_letter(int c, char seq[])
{
    switch (seq[1]) {
    case 'A': return Key_ArrowUp;
    case 'B': return Key_ArrowDown;
    case 'C': return Key_ArrowRight;
    case 'D': return Key_ArrowLeft;
    case 'H': return Key_Home;
    case 'F': return Key_End;
    case 'Z': return Key_ShiftTab;
    default: return c;
    }
}

static int esc_O(int c, char seq[])
{
    switch (seq[1]) {
    case 'H': return Key_Home;
    case 'F': return Key_End;
    case 'P': return Key_F1;
    case 'Q': return Key_F2;
    case 'R': return Key_F3;
    case 'S': return Key_F4;

    default: return c;
    }
}

static int handle_escape_sequence(int c)
{
    // escape sequences travel in pairs of up to 4 bytes

    int fd = fileno(stdin);

    char seq[3] = {0};

    // is it an ordinary ESC?
    if (read(fd, seq, 1) == 0 || read(fd, seq+1, 1) == 0)
        return Key_Esc;

    // it is an escape sequence, check which
    if (seq[0] == '[') {
        if (isdigit(seq[1])) {
            // extended read third byte
            if (read(fd, seq+2, 1) == 0)
                return Key_Esc;

            switch (seq[1]) {
            case '1': return esc_bracket1(c, seq);
            case '2': return esc_bracket2(c, seq);
            case '3': if (seq[2] == '~') return Key_Del;
                break;
            case '5': if (seq[2] == '~') return Key_PgUp;
                break;
            case '6': if (seq[2] == '~') return Key_PgDown;
                break;
            default: return c;
            }
        } else
            return esc_bracket_letter(c, seq);

    } else if (seq[0] == 'O')
        return esc_O(c, seq);

    return c;
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
    wr_ptr += sprintf(wr_ptr, "\x1b[%sm", command);
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

void ui_frm_get_pos(int* x, int* y)
{
    *x = frm_x;
    *y = frm_y;
}

void ui_frm_set_pos(int x, int y)
{
    frm_x = MAX(0, MIN(x, max_cols));
    frm_y = MAX(0, MIN(y, max_rows));
    wr_ptr += sprintf(wr_ptr, "\x1b[%d;%dH", frm_y, frm_x);
}

void ui_printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    wr_ptr += vsnprintf(wr_ptr, BUFFER_SIZE, format, args);
    va_end(args);
}


void ui_printf_at_pos(int x, int y, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    ui_frm_set_pos(x, y);
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
    int y = rows < 0
          ? MAX(1, cursor_y + rows) // up
          : MIN(max_rows, cursor_y + rows);// down

    ui_set_cursor_pos(cursor_x, y);
}

void ui_move_cursor_horz(int cols)
{
    int x = cols < 0
        ? MAX(1, cursor_x + cols) // left
        : MIN(max_cols, cursor_x + cols); // right

    ui_set_cursor_pos(x, cursor_y);
}

void ui_set_cursor_show(bool show)
{
    show_cursor = show;
}


void ui_clear_screen()
{
    update_win_size();

    printf("\x1b[H%s", show_cursor ? "\x1b[?25l" : "");

    for (int i = 0; i < max_rows; ++i) {
        for (int j = 0; j < max_cols; ++j) {
            putc(' ', stdout);
        }
        if (i < max_rows-1)
            putc('\n', stdout);
    }

    printf("\x1b[H%s", show_cursor ? "\x1b[?25h": "");

    fflush(stdout);
}

void ui_render()
{
    if (show_cursor)
        frm_hide_cursor(false);

    // write this frame
    ui_clear_screen();

    frm_top_left();
    printf("%s", buffer);
    printf("\x1b[%d;%dH\x1b[?25h", cursor_y, cursor_x);
    fflush(stdout);

    // from now on every thing is done off screen till next frame render
    frm_reset_buffer();
    if (show_cursor)
        frm_hide_cursor(true);
}

// Restore terminal to normal mode on exit
void ui_disable_raw_mode() {
    update_win_size();
    printf("\x1b[%d;%dH\x1b[0m\n", max_rows, max_cols);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Enable raw mode: disable echo and buffering
void ui_enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(ui_disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Turn off echo and canonical mode
    raw.c_cc[VMIN] = 0; // return each byte, or 0 when timeout
    raw.c_cc[VTIME] = 1; // 100ms wait
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int ui_listen()
{
    // based of kilo editor
    int c;

    if (read(fileno(stdin), &c, 1) > 0) {
        if (c == ESC)
           c = handle_escape_sequence(c);
    }

    return c;
}

int main1() {
    ui_enable_raw_mode();
    int x = 5, y = 5;
    char c;
    (void)frm_top_left;

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