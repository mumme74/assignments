#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include  <signal.h>
#include "arena.h"
#include "controls.h"
#include "document.h"
#include "person.h"
#include "utils.h"
#include "dialog.h"


enum MainPage { FilePage, TestsPage, PersonsPage };
enum FilePageState { NewFile, OpenFile, SaveFile, ExitFile, SaveFileOk };
enum TestPageState { AddTest, EditTest, RemoveTest };
enum PersonPageState { NewPerson, EditPerson, RemovePerson };

static enum MainPage cur_page = FilePage;
static enum FilePageState cur_filepage_state = NewFile;
static enum TestPageState cur_testpage_state = AddTest;
static enum PersonPageState cur_personpage_state = NewPerson;

static const char *cur_file = NULL; //*cur_dir = NULL;
static bool contin = true;

static const char submenu_ids[3][20] = {
    "FileSubmenu", "TestSubmenu", "PersonSubmenu"
};

static mem_Arena global_arena, doc_arena;
static StringArr dir_parts, nav_dir;
static String filename;
static Document *doc = NULL;
static Person cur_user;


static void hide_submenu(ui_Wrapper* menu);
static int submenu_btn_type(ui_Wrapper* btn);
static void handle_file_submenu(ui_Window* win, enum FilePageState state);

// ------------------------------------------------------

// events
void evt_main_menu_btn_click(struct ui_Wrapper* wrap)
{
    if (wrap->type != UI_ButtonType)
        return;

    enum MainPage req = (enum MainPage)wrap->button->name;
    if (req != cur_page) {
        cur_page = req;
        wrap->window->force_redraw = true;
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
    handle_file_submenu(wrap->window, cur_filepage_state);
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

void list_dir_in_nav_dir(struct ui_Wrapper* list, String* text, int row)
{
    struct dirent* de;

    String* path = StringArr_join(&nav_dir, "/", list->window->arena);
    String_append_str(path, "/.", 2);

    DIR* dr = opendir(path->elements);
    if (!dr) {
        String_clear(text);
        return;
    }

    for (int r = 0; r <= row; ++r) {
        de = readdir(dr);
    }

    if (de) {
        String_set(text, de->d_name, strlen(de->d_name));
        if (de->d_type == DT_DIR)
            String_push_back(text, '/');
    }

    closedir(dr);
}

void evt_go_up_one_dir(ui_Wrapper* wrap)
{
    (void)wrap;
    StringArr_pop_back(&nav_dir);

    ui_Wrapper* list = ui_window_get_id(wrap->window, "FileList");
    if (list)
        list->dirty = true;
}

void evt_file_list_clicked(ui_Wrapper* list, int row)
{
    (void)list;
    String text;
    String_init(&text, list->window->arena);

    list_dir_in_nav_dir(list, &text, row);

    if (text.elements[text.size-1] == '/') {
        StringSlice three = String_slice(&text, text.size-3, 3);
        if (three.elements && strncmp(three.elements, "../", 3) == 0) {
            evt_go_up_one_dir(list);
            return;
        }

        StringArr_push_back(&nav_dir, text);

        list->dirty = true;
    } else if (text.size) {
        ui_Wrapper* path = ui_window_get_id(list->window, "FilenameEdit");
        if (!path)
            return;
        ui_control_set_text(path, text.elements, text.size);
    }
}

void evt_file_ok_overwrite(ui_Wrapper* btnOk)
{
    handle_file_submenu(btnOk->window, SaveFileOk);
}


// -------------------------------------------------------------
// menu stuff


static void hide_submenu(ui_Wrapper* menu)
{
    ui_control_set_shown(menu, false);
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
    for (ui_Wrapper* itm = btn->parent->first_child;
         itm != NULL && itm != btn; itm = itm->next_sibling
    )
         state += 1;
    return state;
}

static void update_projectname(ui_Window* win)
{
    if (!doc) return;

    ui_Wrapper* prj_name = ui_window_get_id(win, "ProjectName");
    if (!prj_name) return;

    ui_control_set_text(
        prj_name, doc->project_name.elements, doc->project_name.size);
}

static void create_menu(ui_Window* win)
{
    ui_Wrapper *container = ui_window_new_control(win, UI_ContainerType),
               *lbl       = ui_window_new_control(win, UI_LabelType),
               *menu_btns[] = {
        ui_window_new_control(win, UI_ButtonType),
        ui_window_new_control(win, UI_ButtonType),
        ui_window_new_control(win, UI_ButtonType)
    };

    int cols, rows;
    ui_get_screen_size(&cols, &rows);

    menu_btns[0]->button->name = (const char*)FilePage;
    menu_btns[1]->button->name = (const char*)TestsPage;
    menu_btns[2]->button->name = (const char*)PersonsPage;
    menu_btns[cur_page]->button->bg_color = BackColors.LightCyan;
    ui_window_set_focus(win, menu_btns[cur_page]);

    ui_rect_set(&menu_btns[0]->rect, 3,1, 12,1);
    ui_rect_set(&menu_btns[1]->rect, 15,1, 24,1);
    ui_rect_set(&menu_btns[2]->rect, 27,1, 36,1);
    ui_rect_set(&lbl->rect, 45,1, 79,1);
    ui_rect_set(&container->rect, 0,0, cols,0);

    for (size_t i = 0; i < sizeof(menu_btns)/sizeof(menu_btns[0]); ++i){
        static const char names[3][8] = {"File", "Tests","Persons"};
        String_set(&menu_btns[i]->button->text, names[i], strlen(names[i]));
        menu_btns[i]->button->clicked = evt_main_menu_btn_click;
        menu_btns[i]->button->horz_align = HorzAlignCenter;
        ui_control_append(container, menu_btns[i]);
    }

    lbl->label->format = Format.Dim;
    lbl->id = "ProjectName";
    ui_control_append(container, lbl);

    ui_window_append(win, container);

    update_projectname(win);
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


// -------------------------------------------------------
// Page helpers

static void init_doc(ui_Window* win)
{
    mem_arena_init(&doc_arena);
    doc = (Document*)mem_arena_alloc(&doc_arena, sizeof(Document));
    if (!doc) {
        if (win)
            show_info_dialog(win, "Memory failure", "Failed to allocate");
        else
            write_error("%s", "Memory failure, crating doc");

        return;
    }

    Document_init(doc, &doc_arena);
}

static void new_doc(ui_Window* win)
{
    init_doc(win);
    String_set(&doc->project_name, "Unnamed project", 16);
    if (win) {
        update_projectname(win);
        ui_Wrapper* fname = ui_window_get_id(win, "FilenameEdit");
        if (fname)
            ui_control_set_text(fname, "unnamed", -1);
    }
}

static void load_doc(ui_Window* win, const char* path)
{
    if (!is_file(path)) {
        show_info_dialog(win, "No file", "Please select the file");
        return;
    }

    FILE *fp = fopen(path, "rb");


    if (!fp) {
        show_info_dialog(win, "Failed to open file", NULL);
        return;
    }

    init_doc(win);

    if (!Document_read(doc, fp)) {
        show_info_dialog(win, "Failed to read file", read_error());
        clear_error();
        new_doc(win);
    }

    update_projectname(win);

    fclose(fp);
}

static void save_doc(ui_Window* win, const char* path)
{
    FILE *fp = fopen(path, "wb");
    if (!fp || !Document_write(doc, fp, &cur_user)) {
        show_info_dialog(win, "Failed to write", read_error());
        clear_error();
    } else if (read_warning() && read_warning()[0]) {
        show_info_dialog(win, "Warning", read_warning());
        clear_warning();
    }

    fclose(fp);
}

static void handle_file_submenu(ui_Window* win, enum FilePageState state)
{
    ui_Wrapper* fname = ui_window_get_id(win, "FilenameEdit");
    if (!fname) return;

    String_set_string(&filename, &fname->textedit->text);

    String* path = StringArr_join(&nav_dir, "/", win->arena);
    String_append_str(path, "/", 1);
    String_append_string(path, &filename);

    switch (state) {
    case NewFile:
        new_doc(win);
        break;
    case OpenFile:
        load_doc(win, path->elements);
        break;
    case SaveFile:
        if (!filename.size) {
            show_info_dialog(win, "No filename specified!", NULL);
            return;
        }
        if (access(path->elements, F_OK) == 0) {
            clear_warning();
            write_warning("Should we overwrite?");
            show_ok_cancel_dialog(win,  "File exist", read_warning(),
                                  evt_file_ok_overwrite, NULL);
            clear_warning();
        } else
            save_doc(win, path->elements);

        break;
    case SaveFileOk:
        save_doc(win, path->elements);
        break;
    case ExitFile:
        exit(EXIT_SUCCESS);
        break;
    default:
        break;
    }
}

static void init_current_dirs()
{
    char buf[4096] = {0};
    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    StringArr_init(&dir_parts, &global_arena);
    String_init(&filename, &global_arena);
    String_set(&filename, buf, strlen(buf));
    dir_parts = *String_split(&filename, "/", &global_arena);
    nav_dir = *String_split(&filename, "/", &global_arena);

    String_clear(&filename);
}

// -------------------------------------------------------
// start pages

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
    fname->id = "FilenameEdit";

    ui_Wrapper* up_btn = ui_window_new_control(win, UI_ButtonType);
    ui_control_set_position(up_btn, 1, 3);
    ui_control_set_size(up_btn, 4, 1);
    ui_control_set_text(up_btn, "Up", -1);
    ui_control_append(file_cont, up_btn);
    up_btn->button->clicked = &evt_go_up_one_dir;

    ui_Wrapper* path = ui_window_new_control(win, UI_LabelType);
    ui_control_set_position(path, 6,3);
    ui_control_set_size(path, ui_control_get_width(file_cont)-9, 1);
    ui_control_set_text(path,
        StringArr_join(&nav_dir, "/", path->window->arena)->elements, -1);
    path->label->horz_align = HorzAlignRight;
    ui_control_append(file_cont, path);

    ui_Wrapper* dir_list = ui_window_new_control(win, UI_ListType);
    ui_control_set_position(dir_list, 1,5);
    ui_control_set_size(dir_list,
        ui_control_get_width(file_cont)-2,
        ui_control_get_height(file_cont)-4);
    ui_control_append(file_cont, dir_list);
    dir_list->list->fetch_row = &list_dir_in_nav_dir;
    dir_list->list->select_row_evt = &evt_file_list_clicked;
    dir_list->id = "FileList";
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

    create_menu(win);

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
    create_dialog(win);
}

static void pageloop()
{
    mem_Arena winarena;
    mem_arena_init(&winarena);

    ui_Window win;
    ui_window_init(&win, &winarena);

    create_page(&win, doc);
    enum MainPage page = cur_page;

    while (cur_page == page) {
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

    mem_arena_free(&winarena);
}

void sigint_handler(int no)
{
    (void)no;
    // let ui gracefully restore screen
    exit(EXIT_FAILURE);
}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    signal(SIGINT, sigint_handler);
    catch_output(true);

    mem_arena_init(&global_arena);
    Person_init(&cur_user, &global_arena);
    new_doc(NULL);

    init_current_dirs();

    ui_enable_raw_mode();

    while (contin) {
        pageloop(&doc);

    }

    mem_arena_free(&global_arena);

    ui_disable_raw_mode();

    return EXIT_SUCCESS;
}
