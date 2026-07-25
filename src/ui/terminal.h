#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdint.h>
#include <stddef.h>

#include "typestring.h"

struct _Format
{
    const char *Bold,
               *ResetBold,
               *Dim,
               *RestDim,
               *Underlined,
               *ResetUnderlined,
               *Blink,
               *ResetBlink,
               *Invert,
               *ResetInvert,
               *Hidden,
               *ResetHidden,
               *ResetAll;
};
/// Formating options
extern const struct _Format Format;


/**
 * Colors to render text with
 */
struct _FrontColors {
    const char *Default,
               *Black,
               *Red,
               *Green,
               *Yellow,
               *Blue,
               *Magenta,
               *Cyan,
               *LightGray,
               *DarkGray,
               *LightRed,
               *LightGreen,
               *LightYellow,
               *LightBlue,
               *LightMAgenta,
               *LightCyan,
               *White;
};

/// Colors to render text with
extern const struct _FrontColors FrontColors;


struct _BackColors {
    const char *Default,
               *Black,
               *Red,
               *Green,
               *Yellow,
               *Blue,
               *Magenta,
               *Cyan,
               *LightGray,
               *DarkGray,
               *LightRed,
               *LightGreen,
               *LightYellow,
               *LightBlue,
               *LightMagenta,
               *LightCyan,
               *White;
};

/// Colors to render background with
extern const struct _BackColors BackColors;

// acts on frame
void ui_many_commands(StringArr* commands);
void ui_one_command(const char* command);

void ui_get_screen_size(int* cols, int* rows);

// acts on screen shown
void ui_get_cursor_pos(int* x, int *y);
void ui_set_cursor_pos(int x, int y);
void ui_set_cursor_show(bool show);
void ui_move_cursor_vert(int rows);
void ui_move_cursor_horz(int cols);


void ui_frm_get_pos(int* x, int* y);
void ui_frm_set_pos(int x, int y);
void ui_printf(const char* format, ...);
void ui_printf_at_pos(int x, int y, const char* format, ...);


void ui_render();
void ui_clear_screen();

void ui_disable_raw_mode();
void ui_enable_raw_mode();


#endif // _TERMINAL_H_
