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
    (obj)->wrapper = wrap;

#define INIT_TEXT_INTERFACE(obj, arena) \
    String_init(&(obj)->text, arena); \
    String_set(&(obj)->text, "", 0); \
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
        if (itm == item)
            return;
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
    wrap->shown = true;
    wrap->id = NULL;
}

/// checks if any child is dirty (need repaint)
static bool is_dirty(ui_Wrapper *wrap)
{
    for (; wrap != NULL; wrap = wrap->next_sibling) {
        if (!wrap->shown)
            return false;
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

static ui_Wrapper* lookup_from_id(ui_Wrapper* wrap, const char* id)
{
    for (ui_Wrapper* itm = wrap; itm != NULL; itm = itm->next_sibling) {
        if (itm->id && strcmp(itm->id, id) == 0)
            return itm;
        if (itm->first_child)
            lookup_from_id(itm->first_child, id);
    }
    return NULL;
}

static ui_Wrapper* next_focusable(
    ui_Wrapper* wrap, ui_Wrapper* curobj, bool *after_curobj)
{
    if (!wrap) return NULL;

    ui_Wrapper *itm = NULL,
               *tmp = NULL;

    for (itm = wrap; itm != NULL; itm = itm->next_sibling
    ) {

        if (*after_curobj && ui_control_can_focus(itm) &&
            ui_control_get_enabled(itm) && ui_control_is_visible(itm))
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

        if (*after_curobj && ui_control_can_focus(itm) &&
             ui_control_is_visible(itm) && ui_control_get_enabled(itm))
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
        String* line = &lines->elements[i];
        ui_nprintf_at_pos(x, y+i, line->size, "%s", line->elements);
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
        ui_nprintf_at_pos(x, y, row->size, "%s", row->elements);
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

        ui_nprintf_at_pos(x, y, row->size, "%s", row->elements);
    }

    ui_one_format(bg_window);
}

/**
 * Keeps cursor within 0-max
 */
static void keep_visible(int *scroll, int *cursor, int max)
{
    if (*cursor > max) {
        *scroll += *cursor - max+1;
        *cursor -= *cursor - max+1;
    } else if (*cursor < 0) {
        *scroll += *cursor;
        *cursor -= *cursor;
    }
    *scroll = MAX(0, *scroll);
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
    ui_formats(formats, sizeof(formats)/sizeof(formats[0]));


    StringArr *lines = String_split(&edit->text, "\n", &render_arena);
    if (!lines) return;

    int x = rect->top_left.x,
        y = rect->top_left.y,
        width = rect->bottom_right.x - x,
        height = rect->bottom_right.y - y;

    // Make sure cursor stays within the text
    keep_visible(&edit->scroll.x, &edit->cursor.x, width);
    keep_visible(&edit->scroll.y, &edit->cursor.y, height);

    size_t cur_idx = edit->scroll.y + edit->cursor.y;
    if (cur_idx >= lines->size) {
        edit->cursor.y -= cur_idx - lines->size+1;
        cur_idx = MIN(cur_idx, lines->size)-1;
    }

    // can't move past line len
    int line_len = String_utf8_len(&lines->elements[cur_idx]),
        cur_len = edit->scroll.x + edit->cursor.x;
    if (cur_len >= line_len) {
        edit->cursor.x -= cur_len - line_len;
        keep_visible(&edit->scroll.x, &edit->cursor.x, width);
    }

    // scroll
    if (edit->activated) {
        ui_set_cursor_pos(rect->top_left.x + edit->cursor.x,
                          rect->top_left.y + edit->cursor.y);
        edit->wrapper->window->cursor_holder = edit->wrapper;
    }

    for (int r = edit->scroll.y; y <= rect->bottom_right.y; y++, r++) {

        StringSlice sl = String_uft8_slice(&lines->elements[r], edit->scroll.x, width);
        String* row = String_new_from_slice(&sl, &render_arena);

        size_t prnlen = String_utf8_len(row);
        if (prnlen < (size_t)width)
            String_pad(row, width - prnlen, ' ');

        ui_nprintf_at_pos(x, y, row->size, "%s", row->elements);
    }

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

        if (!ui_control_get_shown(wrap))
            continue;

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

        if (wrap->first_child)
             render(wrap->first_child, focus_ctl, rect.top_left);
    }
}

static void window_render(ui_Window* win)
{
    if (!win->render_cnt++) {
        ui_one_format(bg_window);
        ui_render();
    }

    if (is_dirty(win->root)) {
        ui_Point pnt = {0};
        render(win->root, win->focus_control, pnt);
        ui_set_cursor_show(win->cursor_holder != NULL &&
                           win->cursor_holder == win->focus_control);
        ui_render();
    }
}


static bool list_input(ui_List* list, int c)
{
    switch (c) {
    case Key_Enter:
        if (list->clicked)
            (*list->clicked)(list->wrapper);
        return true;
    default:
        return true;
    }
}

static bool textedit_input(ui_TextEdit* edit, int c)
{
    int line = edit->scroll.y + edit->cursor.y,
        col  = edit->scroll.x + edit->cursor.x;

    // find index in string
    const char *p = edit->text.elements,
               *end = &edit->text.elements[
                    edit->text.size ? edit->text.size : 0];
    {
        int lines = 0, cols = 0;
        for (; p < end && *p != 0 &&  (cols != col || lines != line); ++p) {
            if (*p == '\n')
                lines++;
            else if (lines == line) {
                if ((*p & 0xC0) == 0xC0) {
                    for (++p; p < end && (*p & 0xC0) == 0x80; ++p)
                        ;
                    --p;
                }
                cols++;
            }
        }
    }
    size_t pos = p - edit->text.elements;

    static size_t utf8_rune_len = 0,
                  utf8_rune_idx = 0;
    static char buffer[5] = {0};

    switch (c) {
    case Key_Del: {
        ++p;
        char *prev;
        size_t idx = pos;
        // erase a complete rune
        do {
            if (idx < 1) break;
            prev = &edit->text.elements[--idx];
        } while (((*prev) & 0xC0) == 0x80);

        // can back up
        if (prev < p) {
            StringSlice right = String_slice(&edit->text, pos, -1);
            String *rStr = String_new_from_slice(&right, &render_arena);
            edit->text.size = idx;
            String_append_str(&edit->text, rStr->elements, rStr->size);

            // was it a new line
            if (--edit->cursor.x < 0) {
                size_t mbcnt = 0;
                edit->cursor.y = MAX(edit->cursor.y -1, 0);
                for (p = prev; p+1 > edit->text.elements && *p != '\n'; --p)
                    if ((*p & 0xC0) == 0x80) mbcnt++;
                edit->cursor.x = prev - p - mbcnt - 1;
            }

            edit->wrapper->dirty = true;
        }
    }   return true;
    case Key_Home:
        edit->cursor.x = edit->scroll.x = 0;
        edit->wrapper->dirty = true;
        return true;
    case Key_End: {
        StringArr* lines = String_split(&edit->text, "\n", &render_arena);
        size_t printlen = String_utf8_len(&lines->elements[line]),
               len = edit->wrapper->rect.bottom_right.x
                     - edit->wrapper->rect.top_left.x;
        if (printlen > len) {
            edit->cursor.x = len;
            edit->scroll.x = printlen - len;
        }

        edit->wrapper->dirty = true;
    }   return true;
    case '\n':
        edit->cursor.x = -1;
        edit->cursor.y++;
        // fallthrough
    default:
        if ((c & 0xC0) == 0xC0) {
            // start a multibyte rune
            utf8_rune_len = 0;
            utf8_rune_idx = 0;
            for (size_t i = 4; i < 8; ++i)
                if ((c >> i & 0x01) != 0)
                    utf8_rune_len++;
            buffer[utf8_rune_idx++] = (char)c;
        } else if ((c & 0xC0) == 0x80) {
            buffer[utf8_rune_idx++] = (char)c;
            if (utf8_rune_len == utf8_rune_idx) {
                // insert rune
                StringSlice right = String_slice(&edit->text, pos, -1);
                String *rStr = String_new_from_slice(&right, &render_arena);
                edit->text.size = pos;
                String_append_str(&edit->text, buffer, utf8_rune_idx);
                String_append_str(&edit->text, rStr->elements, rStr->size);
                // clear buffer
                utf8_rune_idx = utf8_rune_len = 0;
                memset(buffer, 0, sizeof(buffer));

                edit->cursor.x++;
                edit->wrapper->dirty = true;
            }
        } else {
            String_insert(&edit->text, (char)c, pos);
            edit->cursor.x++;
            edit->wrapper->dirty = true;
        }
        return true;
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

    //ui_set_cursor_pos(offset_x + cursor->x, offset_y + cursor->y);
}

static bool handle_input(ui_Window* win, int c)
{
    if (ui_window_get_active_state(win) < 1)
        return false;

    switch (win->focus_control->type) {
    case UI_TextEditType:
        return textedit_input(win->focus_control->textedit, c);
    case UI_ListType:
        return list_input(win->focus_control->list, c);
    default:
        return false;
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
        wrap->button->horz_align = HorzAlignCenter;
        wrap->button->vert_align = VertAlignCenter;
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
        INIT_CLICKABLE_INTERFACE(wrap->list)
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
                ui_control_is_visible(tab->control) &&
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
                ui_control_is_visible(tab->control) &&
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

int ui_window_get_active_state(ui_Window* win)
{
    if (!win->focus_control)
        return -1;

    switch (win->focus_control->type) {
    case UI_TextEditType:
        return win->focus_control->textedit->activated;
    case UI_ListType:
        return win->focus_control->list->activated;
    default:
        return -1;
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

ui_Wrapper* ui_window_get_id(ui_Window* win, const char* id)
{
    return lookup_from_id(win->root, id);
}


int ui_window_listen(ui_Window* win)
{
    mem_arena_init(&render_arena);

    window_render(win);

    int c;
    if ((c = ui_listen()) > 0) {
        switch (c) {
        case Key_Enter:
            if (ui_window_get_active_state(win) == 0)
                ui_window_set_active_state(win, true);
            else if (!handle_input(win, c))
                call_click_event(win->focus_control);
            break;
        case Key_Esc:
            if (ui_window_get_active_state(win) == 1)
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

    mem_arena_free(&render_arena);

    return c;
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
    //grow_rect_from_children(cont, &cont->rect);
}

void ui_control_remove(ui_Wrapper* cont, ui_Wrapper* item)
{
    wrap_remove(&cont->first_child, item);
}

bool ui_control_can_focus(ui_Wrapper* wrap)
{
    switch (wrap->type) {
    case UI_ButtonType:
        return wrap->shown && wrap->button->enabled;
    case UI_ListType:
        return wrap->shown && wrap->list->enabled;
    case UI_TextEditType:
        return wrap->shown && wrap->textedit->enabled;

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
    wrap->shown = shown;
    if (shown)
        wrap->dirty = true;
}

bool ui_control_get_shown(ui_Wrapper* wrap)
{
    return wrap->shown;
}

bool ui_control_is_visible(ui_Wrapper* wrap)
{
    if (!wrap)
        return true;
    if (!wrap->shown)
        return false;
    return ui_control_is_visible(wrap->parent);
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

bool ui_control_set_text(ui_Wrapper* wrap, const char* text, int length)
{
    if (!text) return false;

    size_t sz = length > -1 ? (size_t)length : strlen(text);

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

int ui_control_get_width(ui_Wrapper* wrap)
{
    return wrap->rect.bottom_right.x - wrap->rect.top_left.x;
}

int ui_control_get_height(ui_Wrapper* wrap)
{
    return wrap->rect.bottom_right.y - wrap->rect.top_left.y;
}

void ui_control_set_position(ui_Wrapper* wrap, int x, int y)
{
    wrap->rect.top_left.x += x;
    wrap->rect.bottom_right.x += x;
    wrap->rect.top_left.y += y;
    wrap->rect.bottom_right.y += y;
}

void ui_control_set_size(ui_Wrapper* wrap, int width, int height)
{
    int w = ui_control_get_width(wrap),
        h = ui_control_get_height(wrap);
    wrap->rect.bottom_right.x += width - w;
    wrap->rect.bottom_right.y += height - h -1;
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

