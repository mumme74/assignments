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
    int bg_color; \
    struct ui_Wrapper *wrapper;

#define TEXT_INTERFACE \
    String text;         \
    String* reftxt;        \
    enum ui_HorzAlign horz_align; \
    enum ui_VertAlign vert_align; \

#define FORMAT_TEXT_INTERFACE \
    int fg_color, \
        format;

#define FOCUSABLE_INTERFACE \
    int focus_fg_color, \
        focus_bg_color, \
        focus_format; \
    bool enabled;

#define CLICKABLE_INTERFACE \
    ui_EventCb clicked;

#define SCROLLABLE_INTERFACE \
    ui_Point scroll,        \
             cursor;        \
    int active_bg_color, \
        active_fg_color; \
    bool activated;

#define EDITABLE_INTERFACE \
    ui_EventCb changed;


struct ui_Window;
struct ui_Wrapper;

typedef void (*ui_EventCb)(struct ui_Wrapper* wrap);
typedef void (*ui_GetListRow)(struct ui_Wrapper* list, String* text, int row);
typedef void (*ui_RowSelected)(struct ui_Wrapper* list, int row);


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
    FOCUSABLE_INTERFACE
    TEXT_INTERFACE
    FORMAT_TEXT_INTERFACE
    SCROLLABLE_INTERFACE
    EDITABLE_INTERFACE
} ui_TextEdit;

/**
 * A button
 */
typedef struct ui_Button {
    COMMON_INTERFACE
    TEXT_INTERFACE
    FORMAT_TEXT_INTERFACE
    CLICKABLE_INTERFACE
    FOCUSABLE_INTERFACE
} ui_Button;

typedef struct ui_Label {
    COMMON_INTERFACE
    TEXT_INTERFACE
    FORMAT_TEXT_INTERFACE
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
    TEXT_INTERFACE
    SCROLLABLE_INTERFACE
    FORMAT_TEXT_INTERFACE
    CLICKABLE_INTERFACE
    ui_GetListRow fetch_row;
    ui_RowSelected select_row_evt;
} ui_List;

/**
 * Container node for all different controls
 */
typedef struct ui_Wrapper {
    ui_RenderRect rect;
    struct ui_Wrapper *parent, ///< paren node
                       *next_sibling, ///< sibling below
                       *prev_sibling, ///< sibling above
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
    const char *id; ///< a id for this wrapper
    void* data; ///< pointer to arbitrary data

    bool dirty; ///< is should re-render
    bool shown;

} ui_Wrapper;

/**
 * A linked list with the tab order
 */
typedef struct ui_TabOrder {
    ui_Wrapper *control;
    struct ui_TabOrder *next,
                       *prev;
} ui_TabOrder;

/**
 * The rendering window
 */
typedef struct ui_Window {
    ui_Wrapper *root,
               *focus_control; ///< the control that has focus
    ui_TabOrder *first_tab_order;
    mem_Arena *arena;
    size_t render_cnt;
    ui_Wrapper* cursor_holder;
    bool force_redraw;
} ui_Window;

// ----------------------------------------------------------

/**
 * Sets the size of a rectangle
 */
void ui_rect_set(ui_RenderRect* rect, int x1, int y1, int x2, int y2);

/**
 * Set the default colors
 */
void ui_set_default_colors(
    int fg_color, int bg_color, int container_bg,
    int fg_focus_color, int bg_focus_color,
    int bg_window);

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

/**
 * Sets a tabstop for item if its focusable.\
 * If a tabstop is set, it navigates only using that.
 */
bool ui_window_set_taborder(ui_Window* win, ui_Wrapper* item);

/**
 * Navigate to next focusable tabstop
 */
void ui_window_nav_forward(ui_Window* win);

/**
 * Navigate to previous focusable tabstop
 */
void ui_window_nav_backward(ui_Window* win);

/**
 * Set focus to this control, if focusable
 */
void ui_window_set_focus(ui_Window* win, ui_Wrapper* wrap);

/**
 * Gets the active state of currently focused object.
 */
int ui_window_get_active_state(ui_Window* win);


/**
 * Sets the active state of currently focused object.
 */
void ui_window_set_active_state(ui_Window* win, bool active);

/**
 * Looks up the wrapper with matching id
 *
 * @param win The window to look in
 * @param id The id to look for.
 * @return null if failed.
 */
ui_Wrapper* ui_window_get_id(ui_Window* win, const char* id);

/**
 * Forces a redraw, next frame
 */
void ui_window_force_redraw();

/**
 * Start the UI loop
 */
int ui_window_listen(ui_Window* win);


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
 * Test if wrap is visible in the render tree.
 * A Parent might have hidden this control
 */
bool ui_control_is_visible(ui_Wrapper* wrap);

/**
 * Set the active state for those controls that allow it.
 */
void ui_control_set_active(ui_Wrapper* wrap, bool active);

/**
 * Get active state for those controls that implements it
 */
bool ui_control_get_active(ui_Wrapper* wrap);

/**
 * Get text of this control
 */
const String* ui_control_get_text(ui_Wrapper* wrap);

/**
 * Set text of this control
 * @param wrap The controle to set text on
 * @param text The text
 * @param length LEngth if text or -1 for all text
 */
bool ui_control_set_text(ui_Wrapper* wrap, const char* text, int length);

/**
 * Get the dirty status of control.\ Dirty meaning need repaint
 */
bool ui_control_get_dirty(ui_Wrapper* wrap);

/**
 * Set the dirty status of control, where dirty means needing repaint
 */
void ui_control_set_dirty(ui_Wrapper* wrap, bool dirty);

/**
 * Gets the width of the control
 */
int ui_control_get_width(ui_Wrapper* wrap);

/**
 * Gets the height of control
 */
int ui_control_get_height(ui_Wrapper* wrap);

/**
 * Position control, top left
 */
void ui_control_set_position(ui_Wrapper* wrap, int x, int y);

/**
 * Give height and width
 */
void ui_control_set_size(ui_Wrapper* wrap, int width, int height);

/**
 * Gets a bounds rectangle of this wrap grown by its childrens sizes
 */
ui_RenderRect ui_control_get_bounds(ui_Wrapper* wrap);

/**
 * Sets the bounding rect of this control
 */
void ui_control_set_bounds(ui_Wrapper* wrap, ui_RenderRect rect);

#endif
