#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "controls.h"

#include "dialog.h"


static ui_EventCb dialogOk = NULL,
                  dialogCancel = NULL;

static void clear_dialog(ui_Window* win);

// ------------------------------------------------------------
// internal events

void dialog_ok_clicked(ui_Wrapper* btn)
{
    clear_dialog(btn->window);
    if (dialogOk)
        dialogOk(btn);
}

void dialog_cancel_clicked(ui_Wrapper* btn)
{
    clear_dialog(btn->window);
    if (dialogCancel)
        dialogCancel(btn);
}

// ------------------------------------------------------------

static void clear_dialog(ui_Window* win)
{
    ui_Wrapper *dlg = ui_window_get_id(win, "Dialog");
    if (!dlg) return;

    ui_control_set_shown(dlg, false);
}

// ------------------------------------------------------

void show_dialog(
    ui_Window* win, const char* header, const char *message,
    ui_EventCb okCb, ui_EventCb cancelCb, bool show_cancel_btn
) {

    ui_Wrapper *dlg = ui_window_get_id(win, "Dialog"),
               *btnNo = ui_window_get_id(win, "BtnNo"),
               *hdr = ui_window_get_id(win, "DialogHeader"),
               *lbl   = ui_window_get_id(win, "DialogMessage");
    if (!dlg) return;

    if (hdr)
        ui_control_set_text(hdr, header, -1);

    if (lbl)
        ui_control_set_text(lbl, message, -1);

    ui_control_set_shown(dlg, true);

    dialogOk = okCb;
    dialogCancel = cancelCb;

    if (btnNo)
        ui_control_set_shown(btnNo, show_cancel_btn);
}

void show_ok_cancel_dialog(
    ui_Window* win, const char* header, const char* message,
    ui_EventCb evtOk, ui_EventCb evtCancel
) {
    show_dialog(win, header, message, evtOk, evtCancel, true);
}

void show_info_dialog(
    ui_Window* win, const char* header, const char* message
) {
    show_dialog(win, header, message, NULL, NULL, false);
}

void create_dialog(ui_Window* win)
{
    ui_Wrapper *cont = ui_window_new_control(win, UI_ContainerType),
               *header = ui_window_new_control(win, UI_LabelType),
               *lbl = ui_window_new_control(win, UI_LabelType),
               *btnNo = ui_window_new_control(win, UI_ButtonType),
               *btnOk = ui_window_new_control(win, UI_ButtonType);

    ui_window_append(win, cont);
    ui_control_set_shown(cont, false);

    ui_control_set_text(header, "Default header", -1);
    ui_control_set_text(btnOk, "OK", -1);
    ui_control_set_text(btnNo, "Cancel", -1);

    int cols, rows, w = 30, h = 9;
    ui_get_screen_size(&cols, &rows);
    ui_control_set_position(cont, cols/2-w/2, rows/2-h/2);
    ui_control_set_size(cont, w, h);
    cont->id = "Dialog";

    ui_control_set_size(header, w-2, 1);
    ui_control_set_position(header, 1,1);
    header->label->horz_align = HorzAlignCenter;
    header->id = "DialogHeader";
    ui_control_append(cont, header);

    ui_control_set_size(lbl, 28,3);
    ui_control_set_position(lbl, 1,3);
    lbl->label->horz_align = HorzAlignCenter;
    lbl->label->vert_align = VertAlignCenter;
    ui_control_append(cont, lbl);
    lbl->id = "DialogMessage";

    ui_control_set_size(btnOk, 4,1);
    ui_control_set_position(btnOk, 4,7);
    ui_control_append(cont, btnOk);
    btnOk->id = "BtnOk";
    btnOk->button->clicked = dialog_ok_clicked;

    ui_control_set_position(btnNo, 18,7);
    ui_control_set_size(btnNo, 8,1);
    ui_control_append(cont, btnNo);
    btnNo->id = "BtnNo";
    btnNo->button->clicked = dialog_cancel_clicked;
}
