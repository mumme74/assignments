#include <string.h>
#include "typestring.h"
#include "arena.h"
#include "controls.h"

static void wrap_insert(ui_Wrapper **handle, ui_Wrapper *item, int idx)
{
    int i = 0;
    for (ui_Wrapper* itm = *handle; itm != NULL; i++, itm = itm->next_sibling) {
        if (i == idx) {
            item->next_sibling = itm;
            *handle = item;
            return;
        }
        handle = &itm->next_sibling;
    }

    // not found, append last
    *handle = item;
}

static void wrap_append(ui_Wrapper **handle, ui_Wrapper* item)
{
    ui_Wrapper *itm = *handle;
    for (; itm != NULL; itm = itm->next_sibling)
        handle = &itm->next_sibling;

    *handle = item;
}

static void wrap_remove(ui_Wrapper **handle, ui_Wrapper* item)
{
    int i = 0;
    for (ui_Wrapper* itm = *handle; itm != NULL; ++i, itm = itm->next_sibling) {
        if (itm == item) {
            *handle = itm->next_sibling;
            return;
        }
        handle = &itm->next_sibling;
    }
}

static void wrap_init(ui_Wrapper* wrap, ui_Window* win, enum ui_ControlType type)
{
    memset(wrap, 0, sizeof(ui_Wrapper));
    wrap->window = win;
    wrap->type = type;
}

static void button_init(ui_Button* button, ui_Wrapper* wrap)
{
    button->name = "Button";
    button->bg_color = FrontColors.LightGray;
    button->fg_color = FrontColors.Black;
    button->focus_bg_color = BackColors.LightGreen;
    button->focus_fg_color = FrontColors.LightRed;
    button->focus_format = NULL;
    button->format = NULL;
    button->shown = true;
    button->wrapper = wrap;
    String_init(&button->text, wrap->window->arena);
}

static void label_init(ui_Label* label, ui_Wrapper* wrap)
{
    label->name = "Label";
    label->name = NULL;
    label->bg_color = NULL;
    label->fg_color = NULL;
    label->shown = true;
    label->wrapper = wrap;
    String_init(&label->text, wrap->window->arena);
}

static void container_init(ui_Container* cont, ui_Wrapper* wrap)
{
    cont->name = "Container";
    cont->bg_color = NULL;
    cont->shown = true;
    cont->wrapper = wrap;
}

static void textedit_init(ui_TextEdit* textedit, ui_Wrapper* wrap)
{
    textedit->name = "Textedit";
    textedit->bg_color = FrontColors.LightGray;
    textedit->fg_color = FrontColors.Black;
    textedit->focus_bg_color = BackColors.LightGreen;
    textedit->focus_fg_color = FrontColors.LightRed;
    textedit->focus_format = NULL;
    textedit->format = NULL;
    textedit->activated = false;
    textedit->enabled = true;
    textedit->shown = true;
    String_init(&textedit->text, wrap->window->arena);
    textedit->wrapper = wrap;
}

static void list_init(ui_List* list, ui_Wrapper* wrap)
{
    list->name = "List";
    list->bg_color = FrontColors.LightGray;
    list->focus_bg_color = BackColors.LightGreen;
    list->focus_fg_color = FrontColors.LightRed;
    list->focus_format = NULL;
    list->shown = true;
    list->wrapper = wrap;
}


// ---------------------------------------------------------


void ui_window_init(ui_Window* win, mem_Arena* arena)
{
    win->root = NULL;
    win->arena = arena;
}


ui_Wrapper* ui_window_new_control(ui_Window* win, enum ui_ControlType type)
{
    ui_Wrapper* wrap = (ui_Wrapper*)mem_arena_alloc(
                            win->arena, sizeof(ui_Wrapper));

    if (!wrap) return NULL;

    wrap_init(wrap, win, type);

    switch (type) {
    case UI_ButtonType:
        wrap->button = (ui_Button*)mem_arena_alloc(win->arena, sizeof(ui_Button));
        if (!wrap->button) return NULL;
        button_init(wrap->button, wrap);
        break;
    case UI_ContainerType:
        wrap->container = (ui_Container*)mem_arena_alloc(win->arena, sizeof(ui_Container));
        if (!wrap->container) return NULL;
        container_init(wrap->container, wrap);
        break;
    case UI_LabelType:
        wrap->label = (ui_Label*)mem_arena_alloc(win->arena, sizeof(ui_Label));
        if (!wrap->label) return NULL;
        label_init(wrap->label, wrap);
        break;
    case UI_ListType:
        wrap->list = (ui_List*)mem_arena_alloc(win->arena, sizeof(ui_List));
        if (!wrap->list) return NULL;
        list_init(wrap->list, wrap);
        break;
    case UI_TextEditType:
        wrap->textedit = (ui_TextEdit*)mem_arena_alloc(win->arena, sizeof(ui_TextEdit));
        if (!wrap->textedit) return NULL;
        textedit_init(wrap->textedit, wrap);
        break;
    default: return NULL;
    }

    wrap->type = type;

    return wrap;
}


void ui_window_insert(ui_Window* win, ui_Wrapper* item, int idx)
{
    wrap_insert(&win->root, item, idx);
}

void ui_window_append(ui_Window* win, ui_Wrapper* item)
{
    wrap_append(&win->root, item);
}

void ui_window_remove(ui_Window* win, ui_Wrapper* item)
{
    wrap_remove(&win->root, item);
}

void ui_control_insert(ui_Wrapper* cont, ui_Wrapper* item, int idx)
{
    item->parent = cont;
    wrap_insert(&cont->first_child, item, idx);
}

void ui_control_append(ui_Wrapper* cont, ui_Wrapper* item)
{
    item->parent = cont;
    wrap_append(&cont->first_child, item);
}

void ui_control_remove(ui_Wrapper* cont, ui_Wrapper* item)
{
    wrap_remove(&cont->first_child, item);
}

bool ui_control_can_focus(ui_Wrapper* wrap)
{
    switch (wrap->type) {
    case UI_ButtonType:
        return wrap->button->shown && wrap->button->enabled;
    case UI_ListType:
        return wrap->list->shown && wrap->list->enabled;
    case UI_TextEditType:
        return wrap->textedit->shown && wrap->textedit->enabled;

    default: return false;
    }
}

void ui_control_set_enabled(ui_Wrapper* wrap, bool enable)
{
    switch (wrap->type) {
    case UI_ButtonType:
        wrap->button->enabled = enable;
        return;
    case UI_ListType:
        wrap->list->enabled = enable;
        return;
    case UI_TextEditType:
        wrap->textedit->enabled = enable;
        return;
    default: return;
    }
}

bool ui_control_get_enabled(ui_Wrapper* wrap)
{
    switch (wrap->type) {
    case UI_ButtonType:
        return wrap->button->enabled;
    case UI_ListType:
        return wrap->list->enabled;
    case UI_TextEditType:
        return wrap->textedit->enabled;
    default: return false;
    }
}

void ui_control_set_shown(ui_Wrapper* wrap, bool shown)
{
    switch (wrap->type) {
    case UI_ButtonType:
        wrap->button->shown = shown;
        break;
    case UI_ContainerType:
        wrap->container->shown = shown;
        break;
    case UI_LabelType:
        wrap->label->shown = shown;
        break;
    case UI_ListType:
        wrap->list->shown = shown;
        break;
    case UI_TextEditType:
        wrap->textedit->shown = shown;
        break;
    default: return;
    }
}

bool ui_control_get_shown(ui_Wrapper* wrap)
{
   switch (wrap->type) {
    case UI_ButtonType:
        return wrap->button->shown;
    case UI_ContainerType:
        return wrap->container->shown;
    case UI_LabelType:
        return wrap->label->shown;
    case UI_ListType:
        return wrap->list->shown;
    case UI_TextEditType:
        return wrap->textedit->shown;
    default: return false;
    }
}

const String* ui_control_get_text(ui_Wrapper* wrap)
{
   switch (wrap->type) {
    case UI_ButtonType:
        return &wrap->button->text;
    case UI_LabelType:
        return &wrap->label->text;
    case UI_TextEditType:
        return &wrap->textedit->text;
    default: return NULL;
    }
}

bool ui_control_set_text(ui_Wrapper* wrap, const char* text, size_t sz)
{
   switch (wrap->type) {
    case UI_ButtonType:
        return String_set(&wrap->button->text, text, sz);
    case UI_LabelType:
        return String_set(&wrap->button->text, text, sz);
    case UI_TextEditType:
        return String_set(&wrap->button->text, text, sz);
    default: return false;
    }
}
