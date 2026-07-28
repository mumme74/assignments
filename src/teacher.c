#include <stdio.h>
#include <stdlib.h>
#include "arena.h"
#include "controls.h"
#include "document.h"
#include "utils.h"


enum MainPage { FilePage, TestsPage, PersonsPage };
enum FilePageState { NewFile, OpenFile, SaveFile, ExitFile };
enum TestPageState { AddTest, EditTest, RemoveTest };
enum PersonPageState { NewPerson, EditPerson, RemovePerson };

static enum MainPage cur_page = FilePage;
static enum FilePageState cur_filepage_state = NewFile;
static enum TestPageState cur_testpage_state = AddTest;
static enum PersonPageState cur_personpage_state = NewPerson;

static const char *cur_file = NULL; //*cur_dir = NULL;
static bool contin = true, redraw = false;

static const char submenu_ids[3][20] = {
    "FileSubmenu", "TestSubmenu", "PersonSubmenu"
};

static void hide_submenu(ui_Wrapper* menu);
static int submenu_btn_type(ui_Wrapper* btn);

// ------------------------------------------------------

// events
void evt_main_menu_btn_click(struct ui_Wrapper* wrap)
{
    if (wrap->type != UI_ButtonType)
        return;

    enum MainPage req = (enum MainPage)wrap->button->name;
    if (req != cur_page) {
        cur_page = req;
        redraw = true;
        return;
    }


    enum MainPage page = (enum MainPage)wrap->button->name;
    ui_Wrapper *cont = ui_window_get_id(wrap->window, submenu_ids[page]);

    if (cont) {
        bool show = !ui_control_get_shown(cont);
        ui_control_set_shown(cont, show);
        if (show)
            ui_window_set_focus(cont->window, cont->first_child);
    }
}

void evt_file_submenu_click(ui_Wrapper* wrap)
{
    cur_filepage_state = NewFile;
    cur_filepage_state = submenu_btn_type(wrap);
    hide_submenu(wrap->parent);
}

void evt_test_submenu_click(ui_Wrapper* wrap)
{
    cur_testpage_state = AddTest;
    cur_filepage_state = submenu_btn_type(wrap);
    hide_submenu(wrap->parent);
}

void evt_person_submenu_click(ui_Wrapper* wrap)
{
    cur_personpage_state = NewPerson;
    cur_filepage_state = submenu_btn_type(wrap);
    hide_submenu(wrap->parent);
}


// ------------------------------------------------------------

static void hide_submenu(ui_Wrapper* menu)
{
    if (menu->shown)
        redraw = true;
    menu->shown = false;
    menu->dirty = true;
}

static void hide_submenus(ui_Window* win)
{
    for (size_t i = 0; i < sizeof(submenu_ids)/sizeof(submenu_ids[0]); ++i) {
        ui_Wrapper* menu = ui_window_get_id(win, submenu_ids[i]);
        if (menu)
            hide_submenu(menu);
    }
}

static int submenu_btn_type(ui_Wrapper* btn)
{
    int state = 0;
    for (ui_Wrapper* itm = btn;
         itm != NULL && itm != btn; itm = itm->next_sibling
    )
         state += 1;
    return state;
}

static void create_menu(ui_Window* win, const char *filename)
{
    ui_Wrapper *container = ui_window_new_control(win, UI_ContainerType),
               *lbl       = ui_window_new_control(win, UI_ContainerType),
               *menu_btns[] = {
        ui_window_new_control(win, UI_ButtonType),
        ui_window_new_control(win, UI_ButtonType),
        ui_window_new_control(win, UI_ButtonType)
    };

    menu_btns[0]->button->name = (const char*)FilePage;
    menu_btns[1]->button->name = (const char*)TestsPage;
    menu_btns[2]->button->name = (const char*)PersonsPage;
    menu_btns[cur_page]->button->bg_color = BackColors.LightCyan;
    ui_window_set_focus(win, menu_btns[cur_page]);

    ui_rect_set(&menu_btns[0]->rect, 3,1, 12,1);
    ui_rect_set(&menu_btns[1]->rect, 15,1, 24,1);
    ui_rect_set(&menu_btns[2]->rect, 27,1, 36,1);
    ui_rect_set(&lbl->rect, 45,1, 79,1);

    String* fn = String_new(win->arena);
    if (filename)
        String_set(fn, filename, strlen(filename));

    int slash = String_last_index_of(fn, '/');
    if (slash > -1) {
        StringSlice file = String_slice(fn, slash, -1);
        String_set_from_slice(&lbl->label->text, &file);
    } else if (filename)
        String_set(&lbl->label->text, fn->elements, fn->size);

    ui_window_append(win, container);
    for (size_t i = 0; i < sizeof(menu_btns)/sizeof(menu_btns[0]); ++i){
        static const char names[3][8] = {"File", "Tests","Persons"};
        String_set(&menu_btns[i]->button->text, names[i], strlen(names[i]));
        menu_btns[i]->button->clicked = evt_main_menu_btn_click;
        menu_btns[i]->button->horz_align = HorzAlignCenter;
        ui_control_append(container, menu_btns[i]);
    }

    ui_control_append(container, lbl);
}

static void create_sub_menu(
    ui_Window* win, const char *strs[], const char* id,
    size_t str_sz, int xtop, int ytop, int width, ui_EventCb cb
) {
    ui_Wrapper* cont = ui_window_new_control(win, UI_ContainerType);
    ui_window_append(win, cont);
    ui_control_set_shown(cont, false);
    cont->id = id;

    for (size_t i = 0; i < str_sz; ++i) {
        ui_Wrapper* wbtn = ui_window_new_control(win, UI_ButtonType);
        wbtn->button->clicked = cb;
        ui_control_set_position(wbtn, xtop, ytop+i);
        ui_control_set_text(wbtn, strs[i], -1);
        ui_control_set_size(wbtn, width, 1);
        ui_control_append(cont, wbtn);
    }
}

static void create_submenus(ui_Window* win)
{
    switch (cur_page) {
    case FilePage: {
        const char *file_submenus[4] = {"New", "Open", "Save", "Exit"};
        create_sub_menu(win, file_submenus, "FileSubmenu",
            sizeof(file_submenus)/sizeof(file_submenus[0]),
            1,2, 10, evt_file_submenu_click);
    }   break;
    case TestsPage: {
        const char *test_submenus[3] = {"Add", "Edit", "Remove"};
        create_sub_menu(win, test_submenus, "TestSubmenu",
            sizeof(test_submenus)/sizeof(test_submenus[0]),
            15,2, 10, evt_test_submenu_click);

    }   break;
    case PersonsPage: {
        const char *person_submenus[3] = {"New", "Edit", "Remove"};
        create_sub_menu(win, person_submenus, "PersonSubmenu",
            sizeof(person_submenus)/sizeof(person_submenus[0]),
            25,2, 10, evt_test_submenu_click);
    }   break;
    default:
        break;
    }
}

static void create_file_page(ui_Window* win, Document* doc)
{
    (void)doc;
    (void)win;

    int rows, cols;
    ui_get_screen_size(&cols, &rows);

    ui_Wrapper* file_cont = ui_window_new_control(win, UI_ContainerType);
    ui_control_set_position(file_cont, 2,3);
    ui_control_set_size(file_cont, MIN(50, cols), rows-3);
    ui_window_append(win, file_cont);

    ui_Wrapper* wlbl = ui_window_new_control(win, UI_LabelType);
    ui_control_set_position(wlbl, 1,1);
    ui_control_set_size(wlbl, 12,1);
    ui_control_set_text(wlbl, "Project file:", -1);
    ui_control_append(file_cont, wlbl);

    ui_Wrapper* fname = ui_window_new_control(win, UI_TextEditType);
    ui_control_set_position(fname, 15,1);
    ui_control_set_size(fname, ui_control_get_width(file_cont)-17, 1);
    ui_control_set_text(fname, cur_file, -1);
    ui_control_append(file_cont, fname);

    ui_Wrapper* up_btn = ui_window_new_control(win, UI_ButtonType);
    ui_control_set_position(up_btn, 1, 3);
    ui_control_set_size(up_btn, 4, 1);
    ui_control_set_text(up_btn, "Up", -1);
    ui_control_append(file_cont, up_btn);

    ui_Wrapper* dir_list = ui_window_new_control(win, UI_ListType);
    ui_control_set_position(dir_list, 1,5);
    ui_control_set_size(dir_list,
        ui_control_get_width(file_cont)-2,
        ui_control_get_height(file_cont)-4);
    ui_control_append(file_cont, dir_list);
}

static void create_persons_page(ui_Window* win, Document* doc)
{
    (void)doc;
    (void)win;
    ui_Wrapper* wlbl = ui_window_new_control(win, UI_LabelType);
    ui_rect_set(&wlbl->rect, 35,10, 45,15);
    ui_Label *lbl = wlbl->label;
    String_set(&lbl->text, "Persons", 7);
    ui_window_append(win, wlbl);
}


static void create_tests_page(ui_Window* win, Document* doc)
{
    (void)doc;
    (void)win;
    ui_Wrapper* wlbl = ui_window_new_control(win, UI_LabelType);
    ui_rect_set(&wlbl->rect, 35,10, 45,15);
    ui_Label *lbl = wlbl->label;
    String_set(&lbl->text, "Tests", 5);
    ui_window_append(win, wlbl);
}

static void create_page(ui_Window* win, Document* doc)
{

    create_menu(win, cur_file);

    switch (cur_page) {
    case FilePage:
        create_file_page(win, doc);
        break;
    case PersonsPage:
        create_persons_page(win, doc);
        break;
    case TestsPage:
        create_tests_page(win, doc);
        break;
    }

    create_submenus(win);
}

static void pageloop(Document* doc)
{
    (void)doc;
    mem_Arena winarena;
    mem_arena_init(&winarena);

    ui_Window win;
    ui_window_init(&win, &winarena);

    create_page(&win, doc);
    redraw = false;

    while (!redraw) {
        int c;
        if ((c = ui_window_listen(&win)) > 0) {
            switch (c) {
            case Key_Esc:
                hide_submenus(&win);
                break;
            case Key_F1: /* do something clever */ break;
            }
        }
    }
}



int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    mem_Arena docarena;
    Document doc;

    mem_arena_init(&docarena);
    Document_init(&doc, &docarena);

    ui_enable_raw_mode();

    while (contin) {
        pageloop(&doc);

    }

    ui_disable_raw_mode();

    return EXIT_SUCCESS;
}
