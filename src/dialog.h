#ifndef _DIALOG_H_
#define _DIALOG_H_

#include "controls.h"

/**
 * Show a dialog
 * @param win The window object to create it onto
 * @param header The header string
 * @param message The message, may be null
 * @param okCb Callback when ok clicked
 * @param cancelCb Callback when Cancel clicked
 * @param show_cancel_btn If true the cancel button is shown
 */
void show_dialog(
    ui_Window* win, const char* header, const char *message,
    ui_EventCb okCb, ui_EventCb cancelCb, bool show_cancel_Btn
);

/**
 * Show a dialog, Convinience
 * @param win The window object to create it onto
 * @param header The header string
 * @param message The message, may be null
 * @param okCb Callback when ok clicked
 * @param cancelCb Callback when Cancel clicked
 */
void show_ok_cancel_dialog(
    ui_Window* win, const char* header, const char* message,
    ui_EventCb okCb, ui_EventCb cancelCb
);

void show_info_dialog(
    ui_Window* win, const char* header, const char* message
);

void create_dialog(ui_Window* win);

#endif