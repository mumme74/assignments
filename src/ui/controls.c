#include <string.h>
#include <stdlib.h>
#include "typestring.h"
#include "arena.h"
#include "controls.h"
#include "terminal.h"
#include "utils.h"


#define INIT_COMMON_INTERFACE(obj, ctl_name, container) \
    (obj)->name = ctl_name;                  \
    (obj)->bg_color = container ? bg_container : bg_default; \
    (obj)->wrapper = wrap;                   \
    (obj)->shown = true;

#define INIT_TEXT_INTERFACE(obj, arena) \
    String_init(&(obj)->text, arena); \
    (obj)->horz_align = HorzAlignLeft; \
    (obj)->vert_align = VertAlignTop;

#define INIT_FORMAT_TEXT_INTERFACE(obj) \
    (obj)->fg_color = fg_default; \
    (obj)->format = Format.ResetAll;

#define INIT_FOCUSABLE_INTERFACE(obj) \
    (obj)->focus_bg_color = bg_focus_color; \
    (obj)->focus_fg_color = fg_focus_color; \
    (obj)->focus_format = Format.ResetAll; \
    (obj)->enabled = true;

#define INIT_CLICKABLE_INTERFACE(obj) \
    (obj)->clicked = NULL;

#define INIT_SCROLLABLE_INTERFACE(obj) \
    (obj)->active_bg_color = bg_active_color; \
    (obj)->active_fg_color = fg_active_color; \
    (obj)->cursor.x = 0; \
    (obj)->cursor.y = 0; \
    (obj)->scroll.x = 0; \
    (obj)->scroll.y = 0; \
    (obj)->activated = false;

#define INIT_EDITABLE_INTERFACE(obj) \
    (obj)->changed = NULL;

static mem_Arena render_arena;
static int fg_default      = -1,
           bg_default      = -1,
           bg_container    = -1,
           fg_focus_color  = -1,
           bg_focus_color  = -1,
           bg_window       = -1,
           bg_active_color = -1,
           fg_active_color = -1;

static void wrap_insert(ui_Wrapper **handle, ui_Wrapper *item, int idx)
{
    int i = 0;
    ui_Wrapper* itm = *handle, *prev = NULL;
    for (; itm != NULL; i++, itm = itm->next_sibling) {
        if (i == idx) {
            item->next_sibling = itm;
            item->prev_sibling = prev;
            itm->prev_sibling = item;
            *handle = item;
            return;
        }
        handle = &itm->next_sibling;
        prev = itm;
    }

    // not found, append last
    *handle = item;
    item->prev_sibling = prev;
}

static void wrap_append(ui_Wrapper **handle, ui_Wrapper* item)
{
    ui_Wrapper *itm = *handle, *prev = NULL;
    for (; itm != NULL; itm = itm->next_sibling) {
        handle = &itm->next_sibling;
        prev = itm;
    }

    *handle = item;
    item->prev_sibling = prev;
}

static void wrap_remove(ui_Wrapper **handle, ui_Wrapper* item)
{
    int i = 0;
    ui_Wrapper* itm = *handle, *prev = NULL;
    for (; itm != NULL; ++i, itm = itm->next_sibling) {
        if (itm == item) {
            *handle = itm->next_sibling;
            if (itm->next_sibling)
                itm->next_sibling->prev_sibling = prev;
            return;
        }
        handle = &itm->next_sibling;
        prev = itm;
    }
}

static void wrap_init(ui_Wrapper* wrap, ui_Window* win, enum ui_ControlType type)
{
    memset(wrap, 0, sizeof(ui_Wrapper));
    wrap->window = win;
    wrap->type = type;
    wrap->dirty = true;
}

/// checks if any child is dirty (need repaint)
static bool is_dirty(ui_Wrapper *wrap)
{
    for (; wrap != NULL; wrap = wrap->next_sibling) {
        if (wrap->dirty)
            return true;
        if (wrap->first_child && is_dirty(wrap->first_child))
            return true;
    }

    return false;
}

static void grow_rect_from_children(ui_Wrapper* wrap, ui_RenderRect *rect)
{
    for (ui_Wrapper* child = wrap->first_child;
        child != NULL; child = child->next_sibling
    ) {
        if (child->rect.top_left.x < rect->top_left.x)
            rect->top_left.x = child->rect.top_left.x;
        if (child->rect.top_left.y < rect->top_left.y)
            rect->top_left.y = child->rect.top_left.y;
        if (child->rect.bottom_right.x > rect->bottom_right.x)
            rect->bottom_right.x = child->rect.bottom_right.x;
        if (child->rect.bottom_right.y > rect->bottom_right.y)
            rect->bottom_right.y = child->rect.bottom_right.y;

        if (child->first_child)
            grow_rect_from_children(wrap->first_child, rect);
    }
}

static ui_Wrapper* last_sibling(ui_Wrapper* wrap)
{
    ui_Wrapper* tmp = wrap->next_sibling;
    if (!tmp) return wrap;

    for (; tmp->next_sibling != NULL; tmp = tmp->next_sibling) ;

    return tmp;
}

static ui_Wrapper* next_focusable(
    ui_Wrapper* wrap, ui_Wrapper* curobj, bool *after_curobj)
{
    if (!wrap) return NULL;

    ui_Wrapper *itm = NULL,
               *tmp = NULL;

    for (itm = wrap; itm != NULL; itm = itm->next_sibling
    ) {

        if (*after_curobj &&
            ui_control_can_focus(itm) && ui_control_get_enabled(itm))
            return itm;

        if (itm == curobj)
            *after_curobj = true;

        tmp = next_focusable(itm->first_child, curobj, after_curobj);
        if (tmp != NULL)
            return tmp;
    }

    return NULL;
}

static ui_Wrapper* prev_focusable(
    ui_Wrapper* wrap, ui_Wrapper* curobj, bool *after_curobj
) {
    if (!wrap) return NULL;

    ui_Wrapper *itm = NULL,
               *tmp = NULL;

    itm = last_sibling(wrap);

    for (; itm != NULL; itm = itm->prev_sibling
    ) {

        if (*after_curobj &&
            ui_control_can_focus(itm) && ui_control_get_enabled(itm))
            return itm;

        if (itm == curobj)
            *after_curobj = true;

        tmp = prev_focusable(itm->first_child, curobj, after_curobj);
        if (tmp != NULL)
            return tmp;
    }

    return NULL;
}

static void string_row_aligned(
    String* dest, String* src, size_t len, enum ui_HorzAlign h_align
) {
    StringSlice sl = String_uft8_slice(src, 0, len+1);

    switch (h_align) {
    case HorzAlignLeft:
        String_append_from_slice(dest, &sl);
        if (sl.size < len)
            String_pad(dest, len - sl.size, ' ');
        break;
    case HorzAlignRight:
        if (sl.size < len)
            String_pad(dest, len - sl.size, ' ');
        String_append_from_slice(dest, &sl);
        break;
    case HorzAlignCenter: {
        size_t half = (len - sl.size) / 2,
               remain = (len - sl.size) % 2;
        if (sl.size < len)
            String_pad(dest, half, ' ');
        String_append_from_slice(dest, &sl);
        if (sl.size < len)
            String_pad(dest, half+remain, ' ');
       }   break;
    default: break;
    }
}

static StringArr* lines_from_rect(
    String* text, ui_RenderRect* rect,
    enum ui_HorzAlign h_align, enum ui_VertAlign v_align
) {
    size_t len = rect->bottom_right.x - rect->top_left.x,
           rows = rect->bottom_right.y - rect->top_left.y+1;

    StringArr *arr = String_split(text, "\n", &render_arena),
              *lines = StringArr_new(&render_arena);

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
        ssize_t idx = i - renderOffset;

        String* line = String_new(&render_arena);

        if (idx < 0 || idx >= (ssize_t)arr->size)
            String_pad(line, len, ' ');
        else
            string_row_aligned(line, &arr->elements[idx], len, h_align);

        StringArr_push_back(lines, *line);
    }

    return lines;
}

static void draw_lines(StringArr* lines, int x, int y)
{
    for (size_t i = 0; i < lines->size; i++) {
        const char *line = lines->elements[i].elements;
        ui_printf_at_pos(x, y+i, "%s", line);
    }
}

static void render_button(ui_Button *btn, ui_RenderRect* rect, bool has_focus)
{
    int formats[] = {
        has_focus ? btn->focus_format : btn->format,
        has_focus ? btn->focus_fg_color : btn->fg_color,
        has_focus ? btn->focus_bg_color : btn->bg_color,
    };

    StringArr *lines = lines_from_rect(
        &btn->text, rect, btn->horz_align, btn->vert_align);

    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));
    draw_lines(lines, rect->top_left.x, rect->top_left.y);
    ui_one_format(bg_window);
}

static void render_container(ui_Container* cont, ui_RenderRect* rect)
{
    size_t len = rect->bottom_right.x - rect->top_left.x;
    if (len < 1) return;

    String *row = String_new(&render_arena);
    String_pad(row, len, ' ');

    int x = rect->top_left.x, y = rect->top_left.y;

    ui_one_format(cont->bg_color);
    for (; y <= rect->bottom_right.y; y++) {
        ui_printf_at_pos(x, y, "%s", row->elements);
    }

    ui_one_format(bg_window);
}

static void render_label(ui_Label* lbl, ui_RenderRect* rect)
{
    StringArr *lines = lines_from_rect(
        &lbl->text, rect, lbl->horz_align, lbl->vert_align);

    int formats[] = { lbl->format, lbl->bg_color, lbl->fg_color};

    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));
    draw_lines(lines, rect->top_left.x, rect->top_left.y);
    ui_one_format(bg_window);
}

static void render_list(ui_List* list, ui_RenderRect* rect, bool has_focus)
{
    size_t len = rect->bottom_right.x - rect->top_left.x;

    int fg_highlight = list->activated ? list->active_fg_color : list->focus_fg_color,
        bg_highlight = list->activated ? list->active_bg_color : list->focus_bg_color;

    int formats[] = {
        has_focus ? list->focus_format : list->format,
        has_focus ? list->focus_fg_color : list->fg_color,
        has_focus ? list->focus_bg_color : list->bg_color
    };
    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));

    int x = rect->top_left.x,
        y = rect->top_left.y;

    if (list->activated) {
        ui_set_cursor_pos(rect->top_left.x + list->cursor.x,
                          rect->top_left.y + list->cursor.y);
        list->wrapper->window->cursor_holder = list->wrapper;

        if (list->scroll.x < 0) list->scroll.x = 0;
        if (list->scroll.y < 0) list->scroll.y = 0;
    }

    String *row = String_new(&render_arena);

    for (int r = list->scroll.y; y < rect->bottom_right.y; y++, r++) {
        String_clear(row);
        if (list->fetch_row)
            (*list->fetch_row)(list->wrapper, row, r);

        StringSlice sl = String_uft8_slice(row, list->scroll.x, len);
        String* row2 = String_new_from_slice(&sl, &render_arena);
        row = row2 ? row2 : row;

        size_t prnlen = String_utf8_len(row);
        if (prnlen < len)
            String_pad(row, len - prnlen-1, ' ');

        if (list->activated) {
            if (r - list->scroll.y == list->cursor.y) {
                ui_one_format(fg_highlight);
                ui_one_format(bg_highlight);
            } else if (r -1 - list->scroll.y == list->cursor.y) {
                ui_one_format(list->focus_fg_color);
                ui_one_format(list->focus_bg_color);
            }
        }

        ui_printf_at_pos(x, y, "%s", row->elements);
    }

    ui_one_format(bg_window);
}

static void render_textedit(ui_TextEdit* edit, ui_RenderRect* rect, bool has_focus)
{
    int fg_highlight = edit->activated ? edit->active_fg_color : edit->focus_fg_color,
        bg_highlight = edit->activated ? edit->active_bg_color : edit->focus_bg_color;

    int formats[] = {
        has_focus ? edit->focus_format : edit->format,
        has_focus ? fg_highlight : edit->fg_color,
        has_focus ? bg_highlight : edit->bg_color
    };

    if (edit->activated) {
        ui_set_cursor_pos(rect->top_left.x + edit->cursor.x,
                          rect->top_left.y + edit->cursor.y);
        edit->wrapper->window->cursor_holder = edit->wrapper;
    }

    StringArr *lines = lines_from_rect(
        &edit->text, rect, edit->horz_align, edit->vert_align);

    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));
    draw_lines(lines, rect->top_left.x, rect->top_left.y);
    ui_one_format(bg_window);
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
        wrap->dirty = false;

        switch (wrap->type) {
        case UI_ButtonType:
            render_button(wrap->button, &rect, has_focus);
            break;
        case UI_ContainerType:
            render_container(wrap->container, &rect);
            break;
        case UI_LabelType:
            render_label(wrap->label, &rect);
            break;
        case UI_ListType:
            render_list(wrap->list, &rect, has_focus);
            break;
        case UI_TextEditType:
            render_textedit(wrap->textedit, &rect, has_focus);
            break;
        default: break;
        }

        if (wrap->first_child && ui_control_get_shown(wrap))
            render(wrap->first_child, focus_ctl, rect.top_left);
    }
}

static void text_edit_input(ui_TextEdit* edit, int c)
{
    int line = edit->scroll.y + edit->cursor.y,
        col  = edit->scroll.x + edit->cursor.x;
    (void)line;
    (void)col;
    switch (c) {
    case Key_Del:
        break;
    case Key_Home:
        break;
    case Key_End:
        break;
    default: {
    }   break;
    }
}


static void call_click_event(ui_Wrapper* wrap)
{
    if (!wrap) return;

    switch (wrap->type) {
    case UI_ButtonType:
        if (wrap->button->clicked)
            (*wrap->button->clicked)(wrap);
        break;
    default: break;
    }
}

static void move_cursor(ui_Window* win, int x, int y)
{
    if (!win->cursor_holder)
        return;

    ui_Point *cursor,
                *scroll;

    switch (win->cursor_holder->type) {
    case UI_TextEditType:
        cursor = &win->cursor_holder->textedit->cursor;
        scroll = &win->cursor_holder->textedit->scroll;
        break;
    case UI_ListType:
        cursor = &win->cursor_holder->list->cursor;
        scroll = &win->cursor_holder->list->scroll;
        break;
    default:
        return;
    }

    // we got here so we can set cursor
    ui_Point p1 = win->focus_control->rect.top_left,
                p2 = win->focus_control->rect.bottom_right,
            // bottom right wo offset in parent
                pnt = {p2.x -p1.x, p2.y - p1.y};
    if ((cursor->x + x < 0 || cursor->x + x >= pnt.x) ||
        (cursor->y + y < 0 || cursor->y + y >= pnt.y))
    {
        scroll->x += x;
        scroll->y += y;
    } else {
        cursor->x += x;
        cursor->y += y;
    }

    win->cursor_holder->dirty = true;

    int offset_x = 0, offset_y = 0;
    for (ui_Wrapper* itm = win->focus_control;
        itm != NULL; itm = itm->parent
    ) {
        offset_x += itm->rect.top_left.x;
        offset_y += itm->rect.top_left.y;
    }

    ui_set_cursor_pos(offset_x + cursor->x, offset_y + cursor->y);
}

static void handle_input(ui_Window* win, int c)
{
    if (!ui_window_get_active_state(win))
        return;

    switch (win->focus_control->type) {
    case UI_TextEditType:
        text_edit_input(win->focus_control->textedit, c);
        break;
    default: break;
    }
}


// ---------------------------------------------------------


void ui_rect_set(ui_RenderRect* rect, int x1, int y1, int x2, int y2)
{
    rect->top_left.x = x1;
    rect->top_left.y = y1;
    rect->bottom_right.x = x2;
    rect->bottom_right.y = y2;
}

// ----------------------------------------------------------


void ui_window_init(ui_Window* win, mem_Arena* arena)
{
    win->root = NULL;
    win->focus_control = NULL;
    win->first_tab_order = NULL;
    win->arena = arena;
    win->render_cnt = 0;
    win->cursor_holder = NULL;

    if (fg_default < 0) fg_default = FrontColors.Blue;
    if (bg_default < 0) bg_default = BackColors.LightGreen;
    if (bg_container < 0) bg_container = BackColors.DarkGray;
    if (bg_focus_color < 0) bg_focus_color = BackColors.Green;
    if (fg_focus_color < 0) fg_focus_color = BackColors.Cyan;
    if (bg_window < 0) bg_window = BackColors.LightMagenta;
    if (bg_active_color < 0) bg_active_color = BackColors.Cyan;
    if (fg_active_color < 0) fg_active_color = BackColors.Green;
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
        INIT_COMMON_INTERFACE(wrap->button, "Button", false)
        INIT_TEXT_INTERFACE(wrap->button, win->arena)
        INIT_FORMAT_TEXT_INTERFACE(wrap->button)
        INIT_CLICKABLE_INTERFACE(wrap->button)
        INIT_FOCUSABLE_INTERFACE(wrap->button)
        break;
    case UI_ContainerType:
        wrap->container = (ui_Container*)mem_arena_alloc(win->arena, sizeof(ui_Container));
        if (!wrap->container) return NULL;
        INIT_COMMON_INTERFACE(wrap->container, "Container", true)
        wrap->container->bg_color = BackColors.LightGray;
        break;
    case UI_LabelType:
        wrap->label = (ui_Label*)mem_arena_alloc(win->arena, sizeof(ui_Label));
        if (!wrap->label) return NULL;
        INIT_COMMON_INTERFACE(wrap->label, "Label", false)
        INIT_TEXT_INTERFACE(wrap->label, win->arena)
        INIT_FORMAT_TEXT_INTERFACE(wrap->label)
        break;
    case UI_ListType:
        wrap->list = (ui_List*)mem_arena_alloc(win->arena, sizeof(ui_List));
        if (!wrap->list) return NULL;
        INIT_COMMON_INTERFACE(wrap->list, "List", false)
        INIT_FORMAT_TEXT_INTERFACE(wrap->list)
        INIT_FOCUSABLE_INTERFACE(wrap->list)
        INIT_SCROLLABLE_INTERFACE(wrap->list)
        wrap->list->fetch_row = NULL;
        break;
    case UI_TextEditType:
        wrap->textedit = (ui_TextEdit*)mem_arena_alloc(win->arena, sizeof(ui_TextEdit));
        if (!wrap->textedit) return NULL;
        INIT_COMMON_INTERFACE(wrap->textedit, "Textedit", false)
        INIT_TEXT_INTERFACE(wrap->textedit, win->arena)
        INIT_FORMAT_TEXT_INTERFACE(wrap->textedit)
        INIT_FOCUSABLE_INTERFACE(wrap->textedit)
        INIT_SCROLLABLE_INTERFACE(wrap->textedit)
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

bool ui_window_set_taborder(ui_Window* win, ui_Wrapper* item)
{
    if (!ui_control_can_focus(item))
        return false;

    // check if we already have it
    ui_TabOrder **pnext = &win->first_tab_order,
                 *tab = win->first_tab_order,
                 *prev = NULL;
    do {
        if (!tab) break;

        if (tab->control == item)
            return false;

        pnext = &tab->next;
        prev = tab;
        tab = tab->next;
    } while(tab != win->first_tab_order);

    *pnext = (ui_TabOrder*)mem_arena_alloc(win->arena, sizeof(ui_TabOrder));
    if (!*pnext) return false;

    (*pnext)->control = item;
    (*pnext)->next = win->first_tab_order;
    (*pnext)->prev = prev ? prev : *pnext;

    return true;
}

void ui_window_nav_forward(ui_Window* win)
{
    ui_Wrapper **focus_obj = &win->focus_control,
                *tmp = NULL;

    if (win->first_tab_order) {
        ui_TabOrder *tab = win->first_tab_order;
        do {
            if (tab->control != *focus_obj &&
                ui_control_get_enabled(tab->control)
            )
                break;

            tab = tab->next;
        } while (tab != win->first_tab_order);

        ui_window_set_focus(win, tab->control);
        return;
    }

    bool after_curobj = false;

    if (!*focus_obj) {
        after_curobj = true;
        ui_window_set_focus(win, next_focusable(win->root, win->root, &after_curobj));
        return;
    }

    tmp = next_focusable(win->root, *focus_obj, &after_curobj);
    if (!tmp) tmp = next_focusable(win->root, *focus_obj, &after_curobj);
    if (tmp)
         ui_window_set_focus(win, tmp);
}

void ui_window_nav_backward(ui_Window* win)
{
    ui_Wrapper **focus_obj = &win->focus_control,
                *tmp = NULL;
    if (win->first_tab_order) {
        ui_TabOrder *tab = win->first_tab_order;
        do {
            if (tab->control != *focus_obj &&
                ui_control_get_enabled(tab->control)
            )
                break;

            tab = tab->prev;
        } while (tab != win->first_tab_order);

        ui_window_set_focus(win, tab->control);
        return;
    }

    bool after_curobj = false;

    if (!*focus_obj) {
        after_curobj = true;
         ui_window_set_focus(win, prev_focusable(win->root, win->root, &after_curobj));
        return;
    }

    tmp = prev_focusable(win->root, *focus_obj, &after_curobj);
    if (!tmp) tmp = prev_focusable(win->root, *focus_obj, &after_curobj);
    if (tmp)
         ui_window_set_focus(win, tmp);
}

void ui_window_set_focus(ui_Window* win, ui_Wrapper* wrap)
{
    if (wrap && ui_control_can_focus(wrap)) {
        win->focus_control = wrap;
        wrap->dirty = true;
    }
}

bool ui_window_get_active_state(ui_Window* win)
{
    if (!win->focus_control)
        return false;

    switch (win->focus_control->type) {
    case UI_TextEditType:
        return win->focus_control->textedit->activated;
    case UI_ListType:
        return win->focus_control->list->activated;
    default:
        return false;
    }
}

void ui_window_set_active_state(ui_Window* win, bool active)
{
    if (!ui_control_can_focus(win->focus_control))
        return;

    switch (win->focus_control->type) {
    case UI_TextEditType:
        win->focus_control->textedit->activated = active;
        win->focus_control->dirty = true;
        break;
    case UI_ListType:
        win->focus_control->list->activated = active;
        win->focus_control->dirty = true;
        break;
    default: break;
    }
}

int ui_window_listen(ui_Window* win)
{
    ui_window_render(win);

    int c;
    if ((c = ui_listen()) > 0) {
        switch (c) {
        case Key_Enter:
            if (!ui_window_get_active_state(win))
                ui_window_set_active_state(win, true);
            call_click_event(win->focus_control);
            break;
        case Key_Esc:
            if (ui_window_get_active_state(win))
                ui_window_set_active_state(win, false);
            break;
        case Key_Tab:      ui_window_nav_forward(win); break;
        case Key_ShiftTab: ui_window_nav_backward(win); break;
        case Key_ArrowDown:  move_cursor(win, 0, 1); break;
        case Key_ArrowUp:    move_cursor(win, 0,-1); break;
        case Key_ArrowLeft:  move_cursor(win,-1, 0); break;
        case Key_ArrowRight: move_cursor(win, 1, 0); break;
        default:
            handle_input(win, c);
        }
    }

    return c;
}


void ui_window_render(ui_Window* win)
{
    mem_arena_init(&render_arena);
    if (!win->render_cnt++) {
        ui_one_format(bg_window);
        ui_render();
    }

    if (is_dirty(win->root)) {
        ui_Point pnt = {0};
        render(win->root, win->focus_control, pnt);
        ui_set_cursor_show(win->cursor_holder != NULL);
        ui_render();
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
    grow_rect_from_children(cont, &cont->rect);
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

ui_RenderRect ui_control_get_bounds(ui_Wrapper* wrap)
{
    ui_RenderRect rect = wrap->rect;
    grow_rect_from_children(wrap, &rect);
    return rect;
}

void ui_control_set_bounds(ui_Wrapper* wrap, ui_RenderRect rect)
{
    wrap->rect = rect;
    grow_rect_from_children(wrap, &rect);
}

