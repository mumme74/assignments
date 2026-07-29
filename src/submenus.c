#include "controls.h"

#include "submenus.h"


void submenu_set_shown(ui_Wrapper* menu, bool show)
{
    ui_control_set_shown(menu, show);
}

int submenu_btn_nr(ui_Wrapper* btn)
{
    int state = 0;
    for (ui_Wrapper* itm = btn->parent->first_child;
         itm != NULL && itm != btn; itm = itm->next_sibling
    )
         state += 1;
    return state;
}


ui_Wrapper* create_sub_menu(
    ui_Window* win, StringArr* texts, int width, ui_EventCb cb
) {
    ui_Wrapper* cont = ui_window_new_control(win, UI_ContainerType);
    ui_control_set_size(cont, width, texts->size);
    ui_window_append(win, cont);
    ui_control_set_shown(cont, false);

    for (size_t i = 0; i < texts->size; ++i) {
        ui_Wrapper* wbtn = ui_window_new_control(win, UI_ButtonType);
        wbtn->button->clicked = cb;
        ui_control_set_position(wbtn, 0, i);
        ui_control_set_text(wbtn, texts->elements[i].elements, -1);
        ui_control_set_size(wbtn, width, 1);
        ui_control_append(cont, wbtn);
    }

    return cont;
}
