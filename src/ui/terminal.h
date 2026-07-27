#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdint.h>
#include <stddef.h>

#include "typestring.h"

enum Keys {
    Key_Tab = 9,
    Key_Enter = 10,
    Key_Esc = 27,
    Key_Del = 127,
    Key_Home = 10000,
    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,
    Key_End,
    Key_PgDown,
    Key_PgUp,
    Key_Insert,
    Key_ArrowUp,
    Key_ArrowDown,
    Key_ArrowLeft,
    Key_ArrowRight,
    Key_ShiftTab,
};

struct _Format
{
    int Bold,
        ResetBold,
        Dim,
        RestDim,
        Underlined,
        ResetUnderlined,
        Blink,
        ResetBlink,
        Invert,
        ResetInvert,
        Hidden,
        ResetHidden,
        ResetAll;
};
/// Formating options
extern const struct _Format Format;


/**
 * Colors to render text with
 */
struct _FrontColors {
    int Default,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        LightGray,
        DarkGray,
        LightRed,
        LightGreen,
        LightYellow,
        LightBlue,
        LightMAgenta,
        LightCyan,
        White;
};

/// Colors to render text with
extern const struct _FrontColors FrontColors;


struct _BackColors {
    int Default,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        LightGray,
        DarkGray,
        LightRed,
        LightGreen,
        LightYellow,
        LightBlue,
        LightMagenta,
        LightCyan,
        White;
};

/// Colors to render background with
extern const struct _BackColors BackColors;


// acts on frame

/**
 * Sets format and/or color
 */
void ui_formats(int formats[], size_t sz);
void ui_one_format(int format);

void ui_get_screen_size(int* cols, int* rows);

// acts on screen shown
void ui_get_cursor_pos(int* x, int *y);
void ui_set_cursor_pos(int x, int y);
void ui_set_cursor_show(bool show);
void ui_move_cursor_vert(int rows);
void ui_move_cursor_horz(int cols);
void ui_set_scrollable_rows(int start, int end);
void ui_unset_scrollable_rows();


void ui_frm_get_pos(int* x, int* y);
void ui_frm_set_pos(int x, int y);
void ui_printf(const char* format, ...);
void ui_printf_at_pos(int x, int y, const char* format, ...);


void ui_render();
void ui_clear_screen();

int ui_listen();

void ui_disable_raw_mode();
void ui_enable_raw_mode();


#endif // _TERMINAL_H_
