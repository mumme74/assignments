#include <string.h>
#include <stdlib.h>
#include "typestring.h"
#include "arena.h"
#include "controls.h"
#include "terminal.h"
#include "utils.h"


#define INIT_COMMON_INTERFACE(obj, ctl_name) \
    (obj)->name = ctl_name;                  \
    (obj)->bg_color = FrontColors.LightGray; \
    (obj)->wrapper = wrap;                   \
    (obj)->shown = true;

#define INIT_TEXT_INTERFACE(obj, arena) \
    String_init(&(obj)->text, arena); \
    (obj)->horz_align = HorzAlignLeft; \
    (obj)->vert_align = VertAlignTop; \
    (obj)->fg_color = FrontColors.Black;

#define INIT_FOCUSABLE_INTERFACE(obj) \
    (obj)->focus_bg_color = BackColors.LightGreen; \
    (obj)->focus_fg_color = FrontColors.LightRed; \
    (obj)->focus_format = NULL; \
    (obj)->enabled = true;

#define INIT_EDITABLE_INTERFACE(obj) \
    (obj)->enabled = true;

static mem_Arena render_arena;

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

/*
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
    button->horz_align =
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
*/

static bool is_dirty(ui_Wrapper *wrap)
{
    for (; wrap != NULL; wrap = wrap->next_sibling) {
        if (wrap->dirty)
            return true;
    }

    return false;
}

static void string_row_aligned(
    String* str, size_t len, enum ui_HorzAlign h_align
) {
    StringSlice sl = String_uft8_slice(str, 0, len);

    switch (h_align) {
    case HorzAlignLeft:
        String_set_from_slice(str, &sl);
        if (sl.size < len)
            String_pad(str, len - sl.size, ' ');
        break;
    case HorzAlignRight:
        if (sl.size < len)
            String_pad(str, len - sl.size, ' ');
        String_set_from_slice(str, &sl);
        break;
    case HorzAlignCenter:
        if (sl.size < len)
            String_pad(str, (len - sl.size) / 2, ' ');
        String_append_from_slice(str, &sl);
        if (sl.size < len)
            String_pad(str, (len - sl.size) / 2, ' ');
        break;
    default: break;
    }
}

static String* string_from_rect(
    String* text, ui_RenderRect* rect,
    enum ui_HorzAlign h_align, enum ui_VertAlign v_align
) {
    String* str = (String*)mem_arena_alloc(&render_arena, sizeof(String));
    size_t len = rect->bottom_right.x - rect->top_left.x,
           rows = rect->bottom_right.y - rect->top_left.y+1;

    StringArr *arr = String_split(text, "\n", &render_arena);

    int renderOffset = 0;
    switch (v_align){
    case VertAlignBottom:
        renderOffset = rows - MIN(arr->size, rows);
        break;
    case VertAlignCenter:
        renderOffset = (rows - MIN(arr->size, rows)) / 2;
    default: break;
    }

    for (size_t i = 0; i < rows; ++i) {
        if (renderOffset < 1)
            string_row_aligned(str, len, h_align);
    }

    return str;
}

static void render_button(ui_Wrapper *wrap, ui_RenderRect* rect, bool has_focus)
{
    ui_Button *btn = wrap->button;
    const char *formats[] = {
        has_focus ? btn->focus_bg_color : btn->bg_color,
        has_focus ? btn->focus_fg_color : btn->fg_color,
        has_focus ? btn->focus_format : btn->format
    };

    String *str = string_from_rect(
        &btn->text, rect, btn->horz_align, btn->vert_align);

    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));
    ui_printf_at_pos(rect->top_left.x, rect->top_left.y, "%s", str->elements);
    ui_one_format(Format.ResetAll);
}

static void render_container(ui_Wrapper *wrap, ui_RenderRect* rect)
{
    ui_Container *cont = wrap->container;

    ui_one_format(cont->bg_color);
    size_t len = rect->bottom_right.x - rect->top_left.x;

    String row;
    String_init(&row, &render_arena);
    String_pad(&row, len, ' ');

    int x = rect->top_left.x,
        y = rect->top_left.y;
    for (; y <= rect->bottom_right.y; y++) {
        ui_printf_at_pos(x, y, "%s", row.elements);
    }

    ui_one_format(Format.ResetAll);
}

static void render_label(ui_Wrapper* wrap, ui_RenderRect* rect)
{
    ui_Label *lbl = wrap->label;
    const char *formats[] = {
        lbl->bg_color, lbl->fg_color, lbl->format
    };

    String *str = string_from_rect(
        &lbl->text, rect, lbl->horz_align, lbl->vert_align);

    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));
    ui_printf_at_pos(rect->top_left.x, rect->top_left.y, "%s", str->elements);
    ui_one_format(Format.ResetAll);
}

static void render_list(ui_Wrapper* wrap, ui_RenderRect* rect, bool has_focus)
{
    ui_List *list = wrap->list;

    ui_one_format(has_focus ? list->bg_color : list->focus_bg_color);
    size_t len = rect->bottom_right.x - rect->top_left.x;

    String row;
    String_init(&row, &render_arena);
    String_pad(&row, len, ' ');

    int x = rect->top_left.x,
        y = rect->top_left.y;
    for (; y <= rect->bottom_right.y; y++) {
        ui_printf_at_pos(x, y, "%s", row.elements);
    }

    ui_one_format(Format.ResetAll);
}

static void render_textedit(ui_Wrapper* wrap, ui_RenderRect* rect, bool has_focus)
{
    ui_Button *btn = wrap->button;
    const char *formats[] = {
        has_focus ? btn->focus_bg_color : btn->bg_color,
        has_focus ? btn->focus_fg_color : btn->fg_color,
        has_focus ? btn->focus_format : btn->format
    };

    String *str = string_from_rect(
        &btn->text, rect, btn->horz_align, btn->vert_align);

    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));
    ui_printf_at_pos(rect->top_left.x, rect->top_left.y, "%s", str->elements);
    ui_one_format(Format.ResetAll);
}


static void render(ui_Wrapper *wrap, ui_Wrapper* focus_ctl, ui_Point top_left)
{
    for (; wrap != NULL; wrap = wrap->next_sibling) {
        ui_RenderRect rect = {
            {
                .x = wrap->rect.top_left.x + top_left.x,
                .y = wrap->rect.top_left.y + top_left.y
            },
            {
                .x = wrap->rect.bottom_right.x + top_left.x,
                .y = wrap->rect.bottom_right.y + top_left.y
            }
        };

        bool has_focus = wrap == focus_ctl;

        switch (wrap->type) {
        case UI_ButtonType:
            render_button(wrap, &rect, has_focus);
            break;
        case UI_ContainerType:
            render_container(wrap, &rect);
            break;
        case UI_LabelType:
            render_label(wrap, &rect);
            break;
        case UI_ListType:
            render_list(wrap, &rect, has_focus);
            break;
        case UI_TextEditType:
            render_textedit(wrap, &rect, has_focus);
            break;
        default: break;
        }
    }
}


// ---------------------------------------------------------


void ui_window_init(ui_Window* win, mem_Arena* arena)
{
    win->root = NULL;
    win->focus_control = NULL;
    win->first_tab_order = NULL;
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
        INIT_COMMON_INTERFACE(wrap->button, "Button")
        INIT_TEXT_INTERFACE(wrap->button, win->arena)
        INIT_FOCUSABLE_INTERFACE(wrap->button)
        break;
    case UI_ContainerType:
        wrap->container = (ui_Container*)mem_arena_alloc(win->arena, sizeof(ui_Container));
        if (!wrap->container) return NULL;
        INIT_COMMON_INTERFACE(wrap->container, "Container")
        break;
    case UI_LabelType:
        wrap->label = (ui_Label*)mem_arena_alloc(win->arena, sizeof(ui_Label));
        if (!wrap->label) return NULL;
        INIT_COMMON_INTERFACE(wrap->label, "Label")
        INIT_TEXT_INTERFACE(wrap->label, win->arena)
        break;
    case UI_ListType:
        wrap->list = (ui_List*)mem_arena_alloc(win->arena, sizeof(ui_List));
        if (!wrap->list) return NULL;
        INIT_COMMON_INTERFACE(wrap->list, "List")
        INIT_FOCUSABLE_INTERFACE(wrap->list)
        break;
    case UI_TextEditType:
        wrap->textedit = (ui_TextEdit*)mem_arena_alloc(win->arena, sizeof(ui_TextEdit));
        if (!wrap->textedit) return NULL;
        INIT_COMMON_INTERFACE(wrap->textedit, "Textedit")
        INIT_TEXT_INTERFACE(wrap->textedit, win->arena)
        INIT_FOCUSABLE_INTERFACE(wrap->textedit)
        INIT_EDITABLE_INTERFACE(wrap->textedit)
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

void ui_window_render(ui_Window* win)
{
    mem_arena_init(&render_arena);

    if (is_dirty(win->root)) {
        ui_Point pnt = {0};
        render(win->root, win->focus_control, pnt);
    }

    mem_arena_free(&render_arena);
}


// ---------------------------------------------------------------

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
        wrap->dirty = true;
        return String_set(&wrap->button->text, text, sz);
    case UI_LabelType:
        wrap->dirty = true;
        return String_set(&wrap->button->text, text, sz);
    case UI_TextEditType:
        wrap->dirty = true;
        return String_set(&wrap->button->text, text, sz);
    default: return false;
    }
}

void ui_control_set_bounds(ui_Wrapper* wrap, ui_RenderRect rect)
{
    // TODO check that it wont be smaller than any containing controls
    wrap->rect = rect;
}

