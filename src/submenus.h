#ifndef _SUBMENUS_H_
#define _SUBMENUS_H_

#include "controls.h"

/**
 * Hide/show this submenu
 */
void submenu_set_shown(ui_Wrapper* menu, bool show);

/**
 * Get the button nr in this menu
 */
int submenu_btn_nr(ui_Wrapper* menu_btn);

/**
 * Creates a new submenu with buttons as given by
 */
ui_Wrapper* create_sub_menu(
    ui_Window* win, StringArr* texts, int width, ui_EventCb cb);

#endif
