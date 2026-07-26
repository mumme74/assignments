#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "arena.h"
#include "typestring.h"
#include "terminal.h"


#define COMMON_INTERFACE \
    const char *name;    \
    const char *bg_color; \
    struct ui_Wrapper *wrapper; \
    bool shown;

#define TEXT_INTERFACE \
    String text;         \
    enum ui_HorzAlign horz_align; \
    enum ui_VertAlign vert_align; \
    const char *fg_color, \
               *format;

#define FOCUSABLE_INTERFACE \
    const char *focus_fg_color, \
               *focus_bg_color, \
               *focus_format; \
    bool enabled;

#define EDITABLE_INTERFACE \
    bool activated;



struct ui_Window;
struct ui_Wrapper;


/**
 * Allowed controls
 */
enum ui_ControlType {
    UI_ButtonType, UI_ContainerType, UI_TextEditType,
    UI_LabelType, UI_ListType
};

enum ui_HorzAlign {
    HorzAlignLeft, HorzAlignCenter, HorzAlignRight
};

enum ui_VertAlign {
    VertAlignTop, VertAlignCenter, VertAlignBottom
};

/**
 * A x, y point
 */
typedef struct ui_Point {
    int x,
        y;
} ui_Point;

/**
 * The rectangle this controle takes up
 */
typedef struct ui_RenderRect {
    ui_Point top_left,
             bottom_right;
} ui_RenderRect;



/**
 * A Text textedit control
 */
typedef struct ui_TextEdit {
    COMMON_INTERFACE
    TEXT_INTERFACE
    FOCUSABLE_INTERFACE
    EDITABLE_INTERFACE
} ui_TextEdit;

/**
 * A button
 */
typedef struct ui_Button {
    COMMON_INTERFACE
    TEXT_INTERFACE
    FOCUSABLE_INTERFACE
} ui_Button;

typedef struct ui_Label {
    COMMON_INTERFACE
    TEXT_INTERFACE
} ui_Label;

/**
 * A container such as a header
 */
typedef struct ui_Container {
    COMMON_INTERFACE
} ui_Container;

typedef struct ui_List {
    COMMON_INTERFACE
    FOCUSABLE_INTERFACE
} ui_List;

/**
 * Container node for all different controls
 */
typedef struct ui_Wrapper {
    ui_RenderRect rect;
    struct ui_Wrapper *parent, ///< paren node
                       *next_sibling, ///< sibling below
                       *first_child;  ///< fist chile
    struct ui_Window *window; ///< window this is associated with
    union {
        ui_Button *button;
        ui_Container *container;
        ui_List   *list;
        ui_Label  *label;
        ui_TextEdit *textedit;
    };

    enum ui_ControlType type; ///< The type of Control

    bool dirty; ///< is should re-render

} ui_Wrapper;

/**
 * A linked list with the tab order
 */
typedef struct ui_TabOrder {
    ui_Wrapper *control;
    struct ui_TabOrder *next;
} ui_TabOrder;

/**
 * The rendering window
 */
typedef struct ui_Window {
    ui_Wrapper *root,
               *focus_control; ///< the control that has focus
    ui_TabOrder *first_tab_order;
    mem_Arena *arena;
} ui_Window;


/**
 * Initialize a window
 *
 * @param win The window to initialize
 * @param arena The arena used for this window
 */
void ui_window_init(ui_Window* win, mem_Arena* arena);

/**
 * Create a new Control och type, with wrapper
 *
 * @param win The window to create the control to
 * @param type The type of control to create
 * @return The wrapper for this type obj
 */
ui_Wrapper* ui_window_new_control(ui_Window* win, enum ui_ControlType type);

/**
 * Insert item in win at idx pos
 *
 * @param win The window to insert item into
 * @param item The item to insert
 * @param idx At child idx, if -1 then last.
 */
void ui_window_insert(ui_Window* win, ui_Wrapper* item, int idx);

/**
 * Append control to window
 *
 * @param win The window append to
 * @param item The Control wrapper to append
 */
void ui_window_append(ui_Window* win, ui_Wrapper* item);

/**
 * Remove item from window
 *
 * @param win The window to remove item from
 * @param item The item to remove
 */
void ui_window_remove(ui_Window* win, ui_Wrapper* item);

// --------------------------------------------------------s

/**
 * Insert item into the control
 *
 * @param cont The Control to insert item as a child
 * @param item The item to insert
 * @param idx At idx pos, if -1 then last.
 */
void ui_control_insert(ui_Wrapper* cont, ui_Wrapper* item, int idx);

/**
 * Append a wrapper to this
 */
void ui_control_append(ui_Wrapper* cont, ui_Wrapper* item);

/**
 * Remove item from control
 *
 * @param cont The control to remove item from
 * @param item The item to remove.
 */
void ui_control_remove(ui_Wrapper* cont, ui_Wrapper* item);

/**
 * If Control can hold focus
 */
bool ui_control_can_focus(ui_Wrapper* wrap);

/**
 * Sets enabled state
 */
void ui_control_set_enabled(ui_Wrapper* wrap, bool enable);

/**
 * Get enabled state
 */
bool ui_control_get_enabled(ui_Wrapper* wrap);

/**
 * Sets visible state
 */
void ui_control_set_shown(ui_Wrapper* wrap, bool shown);

/**
 * Get visible state
 */
bool ui_control_get_shown(ui_Wrapper* wrap);

/**
 * Get text of this control
 */
const String* ui_control_get_text(ui_Wrapper* wrap);

/**
 * Set text of this control
 */
bool ui_control_set_text(ui_Wrapper* wrap, const char* text, size_t sz);

/**
 * Sets the bounding rect of this control
 */
void ui_control_set_bounds(ui_Wrapper* wrap, ui_RenderRect rect);

#endif
