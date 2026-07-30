#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include  <signal.h>
#include "arena.h"
#include "controls.h"
#include "wrappers_array.h"
#include "document.h"
#include "person.h"
#include "utils.h"
#include "dialog.h"
#include "submenus.h"


enum MainPage { FilePage, TestsPage, PersonsPage };
enum FilePageState { NewFile, OpenFile, SaveFile, ExitFile, SaveFileOk };
enum TestPageState {
    AddTest, EditTest, RemoveTest, AddAtom, EditAtom, RemoveAtom, SelectTest
};
enum PersonPageState { SelectPerson, NewPerson, EditPerson, RemovePerson};

static enum MainPage cur_page = FilePage;
static enum FilePageState cur_filepage_state = NewFile;
static enum TestPageState cur_testpage_state = AddTest;
static enum PersonPageState cur_personpage_state = NewPerson;

static bool contin = true;

static ui_WrapperArr submenus;

static mem_Arena global_arena, doc_arena;
static StringArr dir_parts, nav_dir;
static String filename;
static Document *doc = NULL;
static int cur_person_idx  = -1,
           cur_test_idx = -1,
           cur_atom_idx    = -1;

static void handle_file_submenu(ui_Window* win, enum FilePageState state);
static Person* get_person(int idx);
static TestAtom* get_atom(int idx);
static Test* get_test(int idx);
static void update_person_page_ui(ui_Window *win);
static void update_test_page_ui(ui_Window* win);
static void remove_current_person(ui_Window* win);
static void add_new_person(ui_Window* win);

// ------------------------------------------------------

// events
void evt_main_menu_btn_click(struct ui_Wrapper* btn)
{
    if (btn->type != UI_ButtonType)
        return;

    enum MainPage page = (enum MainPage)btn->data;
    if (page != cur_page) {
        cur_page = page;
        btn->window->force_redraw = true;
        return;
    }

    const char* id = NULL;
    switch (page) {
    case FilePage:    id = "FileSubmenu";    break;
    case TestsPage:   id = "TestsSubmenu";   break;
    case PersonsPage: id = "PersonsSubmenu"; break;
    default: return;
    }

    ui_Wrapper *menu = ui_window_get_id(btn->window, id);
    if (menu) {
        bool show = !ui_control_get_shown(menu);
        ui_control_set_shown(menu, show);
        if (show)
            ui_window_set_focus(menu->window, menu->first_child);
    }
}

void evt_file_submenu_click(ui_Wrapper* wrap)
{
    cur_filepage_state = submenu_btn_nr(wrap);
    submenu_set_shown(wrap->parent, false);
    handle_file_submenu(wrap->window, cur_filepage_state);
}

void evt_test_submenu_click(ui_Wrapper* wrap)
{
    cur_testpage_state = submenu_btn_nr(wrap);
    submenu_set_shown(wrap->parent, false);
}

void evt_person_submenu_click(ui_Wrapper* wrap)
{
    cur_personpage_state = NewPerson + submenu_btn_nr(wrap);
    submenu_set_shown(wrap->parent, false);
    switch (cur_personpage_state) {
    case NewPerson:
        add_new_person(wrap->window);
        // fallthrough
    case EditPerson:
        cur_personpage_state = SelectPerson;
        update_person_page_ui(wrap->window);
        break;
    case RemovePerson:
        remove_current_person(wrap->window);
        break;
    default: break;
    }
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

void list_persons(ui_Wrapper* list, String* text, int row)
{
    (void)list;
    if (!doc || row < 0 || doc->persons.size <= (size_t)row)
        return;

    Person *person = &doc->persons.elements[row];
    String_append_string(text, &person->name);
    String_append_str(text, " <", 2);
    String_append_string(text, &person->email);
    String_append_str(text, "> ", 2);

    StringArr *roles = Person_roles(person, text->arena);
    String_append_string(text, StringArr_join(roles, ",", text->arena));
}

void list_test_row(ui_Wrapper* list, String* text, int row)
{
    (void)list;
    Test* test = get_test(cur_test_idx);
    if (!test || row < 0 || row >= (int)test->atoms.size)
        return;

    TestAtom* atom = &test->atoms.elements[row];

    static const size_t max_type = 5;
    const char* type_name = test_atom_type_to_str(atom->type);
    size_t len = strlen(type_name);
    String_set(text, type_name, MIN(len, max_type));
    if (max_type > len)
        String_pad(text, max_type - len, ' ');

    String_append_string(text, &atom->string);
}

void list_tests(ui_Wrapper* list, String* text, int row)
{
    (void)list;
    if (!doc || row < 0 || row >= (int)doc->test_sessions.size)
        return;

    Test* test = &doc->test_sessions.elements[row];

    String_printf(text, 68, "%2lu atoms, %.20s &",
                  test->atoms.size,
                  test->identifier.elements,
                  test->command);
}


void list_user_roles(ui_Wrapper* list, String* text, int row)
{
    Person *person = get_person(cur_person_idx);
    StringArr *roles = StringArr_new(list->window->arena);

    for (int i = 0; i < DOC_ROLES_BYTE_LEN; ++i) {
        if (person && (person->roles_mask & (0x01 << i)) != 0)
            StringArr_append(roles, person_role_to_str(0x01 << i), -1);
    }

    if ((int)roles->size > row && row > -1)
        String_set_string(text, &roles->elements[row]);
}

void list_all_roles(ui_Wrapper* list, String* text, int row)
{
    Person *person = get_person(cur_person_idx);
    StringArr *roles = StringArr_new(list->window->arena);

    for (int i = 0; i < DOC_ROLES_BYTE_LEN; ++i) {
        if (person && (person->roles_mask & (0x01 << i)) == 0)
            StringArr_append(roles, person_role_to_str(0x01 << i), -1);
    }

    if ((int)roles->size > row && row >-1)
        String_set_string(text, &roles->elements[row]);
}

void list_test_flags(ui_Wrapper* list, String* text , int row)
{
    (void)list;
    Test* test = get_test(cur_test_idx);
    if (!test) return;

    StringArr* flags = Test_flags(test, text->arena);
    if (row > -1 && (int)flags->size > row)
        String_set(text, flags->elements[row].elements,
                         flags->elements[row].size);

}

void list_test_allflags(ui_Wrapper* list, String* text, int row)
{
    (void)list;
    Test* test = get_test(cur_test_idx);
    if (!test || row < 0) return;

    StringArr* available = Test_flags_unset(test, text->arena);

    if (row < (int)available->size)
        String_set_string(text, &available->elements[row]);
}

void list_test_types(ui_Wrapper* list, String* text, int row)
{
    (void)list;
    Test* test = get_test(cur_test_idx);
    if (!test || row < 0 || row >= (int)test->atoms.size)
        return;

    StringArr* types = TestAtom_types(&test->atoms.elements[row], text->arena);
    String_set_string(text, &types->elements[row]);
}

void evt_person_name_changed(ui_Wrapper* edit)
{
    Person* person = get_person(cur_person_idx);
    if (!person) return;

    String_set_string(&person->name, &edit->textedit->text);
    //ui_control_set_dirty(edit, true);
}

void evt_person_email_changed(ui_Wrapper* edit)
{
    Person* person = get_person(cur_person_idx);
    if (!person) return;

    String_set_string(&person->email, &edit->textedit->text);
    //ui_control_set_dirty(edit, true);
}

void evt_person_roles_add(ui_Wrapper* list, int row)
{
    Person* person = get_person(cur_person_idx);
    if (!person) return;

    String *rolestr = String_new(list->window->arena);
    list_all_roles(list, rolestr, row);
    enum PersonRoles role = person_str_to_role(rolestr->elements);

    bool has_changed = person->roles_mask & role;
    ui_control_set_dirty(list, has_changed);

    list->dirty = person->roles_mask |= role;
    person->roles_mask |= role;

    ui_Wrapper* roles_ctl = ui_window_get_id(list->window, "PersonRoles");
    if (roles_ctl)
        ui_control_set_dirty(roles_ctl, true);
}

void evt_person_roles_remove(ui_Wrapper* list, int row)
{
    Person* person = get_person(cur_person_idx);
    if (!person) return;

    String *rolestr = String_new(list->window->arena);
    list_user_roles(list, rolestr, row);
    enum PersonRoles role = person_str_to_role(rolestr->elements);

    bool has_changed = person->roles_mask & role;
    ui_control_set_dirty(list, has_changed);

    person->roles_mask &= ~role;

    ui_Wrapper* allroles_ctl = ui_window_get_id(list->window, "PersonAllRoles");
    if (allroles_ctl)
        ui_control_set_dirty(allroles_ctl, true);
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

void evt_person_selected(ui_Wrapper* list, int row)
{
    (void)list;

    if (!doc || row < 0 || (size_t)row >= doc->persons.size)
        return;

    switch (cur_personpage_state) {
    case SelectPerson:
    case EditPerson:
        cur_person_idx = row;
        cur_personpage_state = EditPerson;
        // show Edit person view
        update_person_page_ui(list->window);
        break;
    case RemovePerson:
        cur_person_idx = -1;
        PersonArr_remove(&doc->persons, row);
        update_person_page_ui(list->window);
        break;
    default:
        update_person_page_ui(list->window);
        break;
    }
}

void evt_remove_person(ui_Wrapper* btnOk)
{
    if (!doc || cur_person_idx < 0 || cur_person_idx >= (int)doc->persons.size)
        return;

    PersonArr_remove(&doc->persons, cur_person_idx);
    --cur_person_idx;

    update_person_page_ui(btnOk->window);
}

void evt_file_ok_overwrite(ui_Wrapper* btnOk)
{
    handle_file_submenu(btnOk->window, SaveFileOk);
}

void evt_test_new_atom(ui_Wrapper* btn)
{
    Test* test = get_test(cur_test_idx);
    if (!test) return;

    cur_atom_idx = test->atoms.size;
    TestAtom atom;
    TestAtom_init(&atom, test->atoms.arena);
    TestAtomArr_push_back(&test->atoms, atom);

    cur_testpage_state = AddAtom;
    update_test_page_ui(btn->window);
}

void evt_remove_test(ui_Wrapper* wrap)
{
    Test* test = get_test(cur_test_idx);
    if (!test) return;

    TestArr_remove(&doc->test_sessions, cur_test_idx--);
    cur_testpage_state = SelectTest;
    update_test_page_ui(wrap->window);
}

void evt_remove_atom(ui_Wrapper* btnOk)
{
    Test* test = get_test(cur_test_idx);
    if (!test || cur_atom_idx < 0 || cur_atom_idx >= (int)test->atoms.size)
        return;

    TestAtomArr_remove(&test->atoms, cur_atom_idx--);
    cur_testpage_state = EditTest;
    update_test_page_ui(btnOk->window);
}

void evt_select_test(ui_Wrapper* list, int row)
{
    Test* test = get_test(row);
    if (!test) return;

    cur_test_idx = row;
    cur_testpage_state = EditTest;
    update_test_page_ui(list->window);
}

void evt_select_atom(ui_Wrapper* list, int row)
{
    Test* test = get_test(cur_test_idx);
    if (!test || row < 0 || row >= (int)test->atoms.size)
        return;

    cur_atom_idx = row;
    cur_testpage_state = EditAtom;
    update_test_page_ui(list->window);
}

void evt_test_remove_flag(ui_Wrapper* list, int row)
{
    Test* test = get_test(cur_test_idx);
    if (!test || row < 0) return;

    String *tmp = String_new(list->window->arena);
    list_test_flags(list, tmp, row);
    if (tmp->size == 0) return;

    enum TestFlags flag = test_flag_str_to_flag(tmp->elements);
    if (!flag) return;

    bool has_changed = Test_clear_flag(test, flag);
    if (!has_changed) return;

    test->flags &= ~flag;

    ui_control_set_dirty(list, true);

    list = ui_window_get_id(list->window, "TestAllflagsList");
    if (list)
        ui_control_set_dirty(list, true);
}

void evt_test_add_flag(ui_Wrapper* list, int row)
{
    Test* test = get_test(cur_test_idx);
    if (!test || row < 0) return;

    String *tmp = String_new(list->window->arena);
    list_test_allflags(list, tmp, row);
    if (tmp->size == 0) return;

    enum TestFlags flag = test_flag_str_to_flag(tmp->elements);
    if (!flag) return;

    bool has_changed = Test_set_flag(test, flag);
    if (!has_changed) return;

    ui_control_set_dirty(list, true);

    list = ui_window_get_id(list->window, "TestFlagsList");
    if (list)
        ui_control_set_dirty(list, true);

}

void evt_show_type_list(ui_Wrapper* wrap)
{
    ui_Wrapper* list = ui_window_get_id(wrap->window, "TestTypeList");
    if (!list) return;

    ui_control_set_shown(list, true);
}

void evt_test_type_selected(ui_Wrapper* list, int row)
{
    ui_control_set_shown(list, false);
    TestAtom* atom = get_atom(cur_atom_idx);
    if (!atom) return;

    atom->type = row;

    ui_Wrapper *btn = ui_window_get_id(list->window, "TestAtomTypeBtn");
    if (!btn) return;

    ui_control_set_text(btn, test_atom_type_to_str(atom->type), -1);
}


// -------------------------------------------------------------
// menu stuff

void hide_submenus()
{
    for (size_t i = 0; i < submenus.size; ++i)
        submenu_set_shown(submenus.elements[i], false);
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

    menu_btns[0]->data = (void*)FilePage;
    menu_btns[1]->data = (void*)TestsPage;
    menu_btns[2]->data = (void*)PersonsPage;
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
        if (i == cur_page)
            menu_btns[i]->button->format = Format.Underlined;
        ui_control_append(container, menu_btns[i]);
    }

    lbl->label->format = Format.Dim;
    lbl->id = "ProjectName";
    ui_control_append(container, lbl);

    ui_window_append(win, container);

    update_projectname(win);
}


static void create_submenus(ui_Window* win)
{
    mem_Arena tmp_arena;
    mem_arena_init(&tmp_arena);

    StringArr texts;
    StringArr_init(&texts, &tmp_arena);

    ui_EventCb clickCb;
    ui_Wrapper* menu;

    switch (cur_page) {
    case FilePage:
        clickCb = evt_file_submenu_click;
        StringArr_append(&texts, "New", -1);
        StringArr_append(&texts, "Open", -1);
        StringArr_append(&texts, "Save", -1);
        StringArr_append(&texts, "Exit", -1);
        menu = create_sub_menu(win, &texts, 10, clickCb);
        menu->id = "FileSubmenu";
        ui_control_set_position(menu, 3, 2);
        break;
    case TestsPage:
        clickCb = evt_test_submenu_click;
        StringArr_append(&texts, "Add", -1);
        StringArr_append(&texts, "Edit", -1);
        StringArr_append(&texts, "Remove", -1);
        menu = create_sub_menu(win, &texts, 10, clickCb);
        menu->id = "TestsSubmenu";
        ui_control_set_position(menu, 15, 2);
        break;
    case PersonsPage:
        clickCb = evt_person_submenu_click;
        StringArr_append(&texts, "New", -1);
        StringArr_append(&texts, "Edit", -1);
        StringArr_append(&texts, "Remove", -1);
        menu = create_sub_menu(win, &texts, 10, clickCb);
        menu->id = "PersonsSubmenu";
        ui_control_set_position(menu, 27, 2);
        break;
    default:
        return;
    }

    ui_WrapperArr_push_back(&submenus, menu);

    mem_arena_free(&tmp_arena);
}

// -------------------------------------------------------
// Page helpers

static Person* get_person(int idx)
{
    if (!doc || idx < 0|| (size_t)idx >= doc->persons.size)
        return NULL;

    return &doc->persons.elements[idx];
}

static Test* get_test(int idx)
{
    if (!doc || idx < 0 || (size_t)idx >= doc->test_sessions.size)
        return NULL;
    return &doc->test_sessions.elements[idx];
}

static TestAtom* get_atom(int idx)
{
    Test* test = get_test(cur_test_idx);
    if (!test || idx < 0 || (size_t)idx >= test->atoms.size)
        return NULL;
    return &test->atoms.elements[idx];
}

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
    cur_person_idx = -1;
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
        if (win)
            show_info_dialog(win, "No file", "Please select the file");
        return;
    }

    FILE *fp = fopen(path, "rb");


    if (!fp) {
        if (win)
            show_info_dialog(win, "Failed to open file", NULL);
        return;
    }

    init_doc(win);

    if (!Document_read(doc, fp)) {
        if (win) {
            show_info_dialog(win, "Failed to read file", read_error());
            clear_error();
        }
        new_doc(win);
    }

    if (doc->persons.size)
        cur_person_idx = doc->header.compiler_person;

    if (doc->test_sessions.size)
        cur_testpage_state = SelectTest;

    if (win)
        update_projectname(win);

    fclose(fp);
}

static void save_doc(ui_Window* win, const char* path)
{
    FILE *fp = fopen(path, "wb");

    Person* person = get_person(cur_person_idx);
    if (!person) return;

    if (!fp || !Document_write(doc, fp, person)) {
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

static void add_new_person(ui_Window* win)
{
    Person new_person;
    Person_init(&new_person, doc->persons.arena);
    cur_person_idx = doc->persons.size;
    if (!PersonArr_push_back(&doc->persons, new_person))
        show_info_dialog(win, "Mem error", "Failed to add new person");

    cur_personpage_state = NewPerson;
}

static void remove_current_person(ui_Window* win)
{
    Person* person = get_person(cur_person_idx);
    if (!person) return;

    show_ok_cancel_dialog(win, "Remove user",
        "Are You sure you want to remove user?",
         evt_remove_person, NULL);
}

static void set_person_form(ui_Window* win)
{
    ui_Wrapper *form = ui_window_get_id(win, "PersonEditForm"),
               *name = ui_window_get_id(win, "PersonNameEdit"),
               *email = ui_window_get_id(win, "PersonEmailEdit");

    if (!doc || !form || !name || !email)
        return;

    if (cur_person_idx < 0)
        add_new_person(win);

    Person *user = &doc->persons.elements[cur_person_idx];

    String_set_string(&name->textedit->text, &user->name);
    String_set_string(&email->textedit->text, &user->email);
}

static void set_enable_form(ui_Wrapper* form, bool enable)
{
    for (ui_Wrapper* itm = form; itm != NULL; itm = itm->next_sibling) {
        ui_control_set_enabled(itm, enable);
    }
}

static void set_info_msg(ui_Window* win, const char* message)
{
    ui_Wrapper* info = ui_window_get_id(win, "InfoLabel");
    if (info) {
        ui_control_set_text(info, message, -1);
        bool shown = message && *message != '\0';
        ui_control_set_shown(info, shown);
    }
}

static void update_person_page_ui(ui_Window *win)
{
    if (!doc || cur_page != PersonsPage)
        return;

    ui_Wrapper *select = ui_window_get_id(win, "PersonSelector"),
               *form =  ui_window_get_id(win, "PersonEditForm");

    bool enable = true;
    const char* msg = "";

    switch (cur_personpage_state) {
    case NewPerson:
        enable = false;
        msg = "Need to add a new user first!";
        // fallthrough
    case EditPerson:
        ui_control_set_shown(select, false);
        ui_control_set_shown(form, true);
        set_enable_form(form, enable);
        set_person_form(win);
        break;
    case RemovePerson:
    case SelectPerson:
        ui_control_set_shown(select, true);
        ui_control_set_shown(form, false);
        break;
    }
    set_info_msg(win, msg);
}

static void update_test_page_ui(ui_Window* win)
{
    if (!doc || cur_page != TestsPage)
        return;

    const char *ids[3] = {
        "TestSelector", "TestEditForm", "TestAtomEditForm"
    };

    bool enable = true;
    const char *msg = "",
               *shown_id;

    switch (cur_testpage_state) {
    case RemoveTest:
    case SelectTest:
        shown_id = ids[0];
        enable = true;
        break;
    case AddTest:
        enable = false;
        // fallthrough
    case RemoveAtom:
    case EditTest:
        shown_id = ids[1];
        break;
    case AddAtom:
        enable = false;
        // fallthrough
    case EditAtom:
        shown_id = ids[2];
        break;
    default:
        return;
    }

    for (size_t i = 0; i < sizeof(ids)/sizeof(ids[0]); i++) {
        ui_Wrapper* cont = ui_window_get_id(win, ids[i]);
        if (cont) {
            if (ids[i] == shown_id) {
                ui_control_set_shown(cont, true);
                set_enable_form(cont, enable);
            } else
                ui_control_set_shown(cont, false);
        }
    }

    set_info_msg(win, msg);
}

// --------------------------------------------------------------
// Page parts

ui_Wrapper* create_person_selector(ui_Window* win, ui_RowSelected selected)
{
    int cols, rows, w = 70, h = 15;
    ui_get_screen_size(&cols, &rows);

    int width = MIN(w, cols-2),
        height = MAX(h,rows-3);


    ui_Wrapper* sel_cont = ui_window_new_control(win, UI_ContainerType);
    ui_rect_set(&sel_cont->rect, 2,3, width,height);
    ui_window_append(win, sel_cont);
    sel_cont->id = "PersonSelector";

    ui_Wrapper *sel_lbl = ui_window_new_control(win, UI_LabelType);
    ui_control_set_text(sel_lbl, "Select Person", 14);
    ui_rect_set(&sel_lbl->rect, 1,1, 14,1);
    ui_control_append(sel_cont, sel_lbl);

    ui_Wrapper *sel_list = ui_window_new_control(win, UI_ListType);
    ui_rect_set(&sel_list->rect, 1,3, width-2,height-2);
    sel_list->id = "SelectPersonList";
    sel_list->list->fetch_row = list_persons;
    sel_list->list->select_row_evt = selected;
    ui_control_append(sel_cont, sel_list);

    return sel_cont;
}

static ui_Wrapper* create_person_form(ui_Window* win)
{
    int cols, rows, w = 70, h = 15;
    ui_get_screen_size(&cols, &rows);

    int width = MIN(w, cols-2),
        height = MAX(h, rows-3);

    ui_Wrapper *cont      = ui_window_new_control(win, UI_ContainerType),
               *name      = ui_window_new_control(win, UI_TextEditType),
               *email     = ui_window_new_control(win, UI_TextEditType),
               *lbl_name  = ui_window_new_control(win, UI_LabelType),
               *lbl_email = ui_window_new_control(win, UI_LabelType),
               *roles     = ui_window_new_control(win, UI_ListType),
               *all_roles = ui_window_new_control(win, UI_ListType);

    if (!cont || !name || !email || !lbl_email ||
        !lbl_name || !roles || !all_roles
    )
        return NULL;

    ui_rect_set(&cont->rect, 2,3, width,height);
    ui_window_append(win, cont);
    cont->id = "PersonEditForm";

    ui_control_set_text(lbl_name, "Name", 4);
    ui_rect_set(&lbl_name->rect, 1,1, 5,1);
    ui_control_append(cont, lbl_name);

    name->id = "PersonNameEdit";
    name->textedit->changed = evt_person_name_changed;
    ui_rect_set(&name->rect, 7,1, width-3,1);
    ui_control_append(cont, name);

    ui_control_set_text(lbl_email, "Email", 5);
    ui_rect_set(&lbl_email->rect, 1,3, 5,3);
    ui_control_append(cont, lbl_email);

    email->id = "PersonEmailEdit";
    email->textedit->changed = evt_person_email_changed;
    ui_rect_set(&email->rect, 7,3, width-3,3);
    ui_control_append(cont, email);

    roles->id = "PersonRoles";
    roles->list->fetch_row = list_user_roles;
    roles->list->select_row_evt = evt_person_roles_remove;
    ui_rect_set(&roles->rect, 1,5, width/2-1,height-3);
    ui_control_append(cont, roles);

    all_roles->id = "PersonAllRoles";
    all_roles->list->fetch_row = list_all_roles;
    all_roles->list->select_row_evt = evt_person_roles_add;
    ui_rect_set(&all_roles->rect, width/2+1,5, width-2,height-3);
    ui_control_append(cont, all_roles);

    return cont;
}

static ui_Wrapper* create_test_selector(ui_Window* win)
{
    ui_Wrapper *cont     = ui_window_new_control(win, UI_ContainerType),
               *lbl      = ui_window_new_control(win, UI_LabelType),
               *btn_new  = ui_window_new_control(win, UI_ButtonType),
               *list     = ui_window_new_control(win, UI_ListType);
    if (!cont || !lbl || !list || !btn_new)
        return NULL;

    int rows, cols, w = 70, h = 15;
    ui_get_screen_size(&cols, &rows);

    int right = MIN(w, cols) - 2,
        bottom = MAX(h, rows) - 3;

    cont->id = "TestSelector";
    ui_rect_set(&cont->rect, 2,3, right+2,bottom+2);
    ui_window_append(win, cont);

    lbl->id = "TestSelectorLbl";
    ui_control_set_text(lbl, "Select test", 11);
    ui_rect_set(&lbl->rect, 1,1, 12,1);
    ui_control_append(cont, lbl);

    btn_new->id = "TestSelectorNew";
    ui_control_set_text(btn_new, "New", 3);
    ui_rect_set(&btn_new->rect, right-6,1, right-1,1);
    ui_control_append(cont, btn_new);

    list->id = "TestSelectorList";
    list->list->fetch_row = list_tests;
    list->list->select_row_evt = evt_select_test;
    ui_rect_set(&list->rect, 1,3, right-1,bottom-1);
    ui_control_append(cont, list);

    return cont;

}

static ui_Wrapper* create_test_edit(ui_Window* win)
{
    ui_Wrapper *cont     = ui_window_new_control(win, UI_ContainerType),
               *lbl_name = ui_window_new_control(win, UI_LabelType),
               *name     = ui_window_new_control(win, UI_TextEditType),
               *lbl_cmd  = ui_window_new_control(win, UI_LabelType),
               *cmd      = ui_window_new_control(win, UI_TextEditType),
               *btn_type = ui_window_new_control(win, UI_ButtonType),
               *list_type  = ui_window_new_control(win, UI_ListType),
               *btn_new  = ui_window_new_control(win, UI_ButtonType),
               *lbl_flags  = ui_window_new_control(win, UI_LabelType),
               *list_flags = ui_window_new_control(win, UI_ListType),
               *lbl_allflags  = ui_window_new_control(win, UI_LabelType),
               *list_allflags = ui_window_new_control(win, UI_ListType),
               *list     = ui_window_new_control(win, UI_ListType);

    if (!cont || !lbl_name || !name || !lbl_cmd || !cmd || !btn_type ||
        !btn_type || !lbl_flags || !list_flags || !lbl_allflags ||
        !list_allflags || !list
    )
        return NULL;

    int cols, rows, w = 70, h = 17;
    ui_get_screen_size(&cols, &rows);

    int right  = MIN(w, cols)-4,
        bottom = MAX(h, rows)-3;

    cont->id = "TestEditForm";
    ui_rect_set(&cont->rect, 2,3, right+2,bottom+2);
    ui_window_append(win, cont);

    lbl_name->id = "TestNameLbl";
    ui_control_set_text(lbl_name, "Testsession name", 16);
    ui_rect_set(&lbl_name->rect, 1,0, 17,0);
    ui_control_append(cont, lbl_name);

    name->id = "TestNameEdit";
    ui_rect_set(&name->rect, 1,1, 30,1);
    ui_control_append(cont, name);

    lbl_cmd->id = "TestCmdLbl";
    ui_control_set_text(lbl_cmd, "OS command", 11);
    ui_rect_set(&lbl_cmd->rect, 1,3, 12,3);
    ui_control_append(cont, lbl_cmd);

    cmd->id = "TestCmdEdit";
    ui_rect_set(&cmd->rect, 1,4, 30,4);
    ui_control_append(cont, cmd);

    btn_type->id = "TestTypeBtn";
    btn_type->button->clicked = evt_show_type_list;
    ui_control_set_text(btn_type, "Type", 4);
    ui_rect_set(&btn_type->rect, 32,0, 38,0);
    ui_control_append(cont, btn_type);

    btn_new->id = "TestNewBtn";
    btn_new->button->clicked = evt_test_new_atom;
    btn_new->button->horz_align = HorzAlignCenter;
    ui_rect_set(&btn_new->rect, right-6,0, right-1, 0);
    ui_control_set_text(btn_new, "New", 5);
    ui_control_append(cont, btn_new);

    list->id ="TestTestsList";
    list->list->fetch_row = list_test_row;
    list->list->select_row_evt = evt_select_atom;
    ui_rect_set(&list->rect, 32,2, right,bottom-1);
    ui_control_append(cont, list);

    lbl_flags->id = "TestFlagsLbl";
    ui_control_set_text(lbl_flags, "Has states", -1);
    ui_rect_set(&lbl_flags->rect, 1,6, 10,6);
    ui_control_append(cont,  lbl_flags);

    list_flags->id = "TestFlagsList";
    list_flags->list->fetch_row = list_test_flags;
    list_flags->list->select_row_evt = evt_test_remove_flag;
    ui_rect_set(&list_flags->rect, 1,7, 13,12);
    ui_control_append(cont, list_flags);

    lbl_allflags->id = "TestAllflagsLbl";
    ui_control_set_text(lbl_allflags, "Available", 10);
    ui_rect_set(&lbl_allflags->rect, 15,6, 24,6);
    ui_control_append(cont, lbl_allflags);

    list_allflags->id = "TestAllflagsList";
    list_allflags->list->fetch_row = list_test_allflags;
    list_allflags->list->select_row_evt = evt_test_add_flag;
    ui_rect_set(&list_allflags->rect, 15,7, 30,13);
    ui_control_append(cont, list_allflags);

    // insert last to render last as we don't have any Z-order
    list_type->id = "TestTypeList";
    list_type->list->fetch_row = list_test_types;
    list_type->list->select_row_evt = evt_test_type_selected;
    ui_control_set_shown(list_type, false);
    ui_control_set_position(list_type,
        btn_type->rect.top_left.x, btn_type->rect.top_left.y+1);
    ui_control_set_size(list_type, 10, 5);
    ui_control_append(cont, list_type);


    return cont;
 }

 static ui_Wrapper* create_atom_edit(ui_Window* win)
 {
    ui_Wrapper *cont      = ui_window_new_control(win, UI_ContainerType),
               *test_id   = ui_window_new_control(win, UI_LabelType),
               *lbl_str   = ui_window_new_control(win, UI_LabelType),
               *edit_str  = ui_window_new_control(win, UI_TextEditType);

    if (!cont || !test_id || !lbl_str || !edit_str)
        return NULL;

    int cols, rows, w = 70, h = 15;
    ui_get_screen_size(&cols, &rows);

    int width = MIN(w, cols)-2,
        height = MAX(h, rows)-3;

    cont->id = "TestAtomEditForm";
    ui_rect_set(&cont->rect, 2,3, width+1,height+1);
    ui_window_append(win, cont);

    test_id->id = "TestAtomTestId";
    test_id->label->bg_color = cont->container->bg_color;
    ui_control_set_text(test_id, "Testcase", -1);
    ui_rect_set(&test_id->rect, 1,0, width-2,0);
    ui_control_append(cont, test_id);

    lbl_str->id = "TestAtomStrLbl";
    ui_rect_set(&lbl_str->rect, 13,1, 19,1);
    ui_control_set_text(lbl_str, "String", -1);
    ui_control_append(cont, lbl_str);

    edit_str->id = "TestAtomStrEdit";
    ui_rect_set(&edit_str->rect, 13,3, width-15,3);
    ui_control_append(cont, edit_str);

    return cont;
 }

 static ui_Wrapper* create_info_lbl(ui_Window* win)
 {
    int rows, cols;
    ui_get_screen_size(&cols, &rows);

    ui_Wrapper* info = ui_window_new_control(win, UI_LabelType);
    info->label->bg_color = BackColors.Cyan;
    info->label->fg_color = FrontColors.Red;
    info->id = "InfoLabel";
    ui_rect_set(&info->rect, 1,2, MIN(70, cols-2),2);
    ui_control_set_shown(info, false);
    ui_window_append(win, info);

    return info;
 }

// -------------------------------------------------------
// start pages

static void create_file_page(ui_Window* win)
{
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
    ui_control_set_text(fname, filename.elements, filename.size);
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

static void create_persons_page(ui_Window* win)
{
    ui_Wrapper* sel = create_person_selector(win, evt_person_selected);
    if (sel) ui_window_append(win, sel);

    ui_Wrapper* edit = create_person_form(win);
    if (edit) ui_window_append(win, edit);

    ui_Wrapper* cont = ui_control_get_shown(edit) ? edit : sel;
    if (cont) create_info_lbl(cont->window);

    update_person_page_ui(win);
}


static void create_tests_page(ui_Window* win)
{
    create_test_edit(win),
    create_atom_edit(win),
    create_test_selector(win);


    create_info_lbl(win);

    update_test_page_ui(win);
}

static void create_page(ui_Window* win)
{
    create_menu(win);

    switch (cur_page) {
    case FilePage:
        create_file_page(win);
        break;
    case PersonsPage:
        create_persons_page(win);
        break;
    case TestsPage:
        create_tests_page(win);
        break;
    }

    create_submenus(win);
    create_dialog(win);
}

static void pageloop()
{
    mem_Arena page_arena;
    mem_arena_init(&page_arena);

    ui_Window win;
    ui_window_init(&win, &page_arena);

    ui_WrapperArr_init(&submenus, &page_arena);

    create_page(&win);
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

    mem_arena_free(&page_arena);
}

// ------------------------------------------------------

void sigint_handler(int no)
{
    (void)no;
    // let ui gracefully restore screen
    exit(EXIT_FAILURE);
}

//--------------------------------------------------------

int print_help(const char* prgm) {
    printf("usage %s [options] <file to test>", prgm);
    printf("\n file to test might be omitted to enter UI mode\n"
           " -h        Show help\n"
           " --help  \n"
           " --doc        path to testdoc file\n");
    return 0;
}

static int parse_argv(int argc, const char *argv[], const char **testdoc_path)
{
    (void)testdoc_path;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 ||
            strcmp(argv[i], "--help") == 0
        ) {
            print_help(argv[0]);
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "--doc") == 0) {
            if (i == argc-1) {
                fprintf(stderr,
                    "\x1b[31mError: Must give filepath to testdoc!\n\x1b[41m");
                exit(EXIT_FAILURE);
            }

            testdoc_path = &argv[++i];
        }

        if (argc-1 == i && argv[i][0] != '-') {
            return 1;
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    const char* testdoc_path = NULL;
    int res = parse_argv(argc, argv, &testdoc_path);

    signal(SIGINT, sigint_handler);
    catch_output(true);

    mem_arena_init(&global_arena);
    new_doc(NULL);

    init_current_dirs();

    load_doc(NULL, testdoc_path ? testdoc_path : ".testdoc");

    if (res == 0) {
        // enter UI mode
        ui_enable_raw_mode();

        while (contin) {
            pageloop(&doc);
        }

    } else {
        // enter direct mode
        printf("TODO: finish direct mode\n");
    }

    mem_arena_free(&global_arena);

    ui_disable_raw_mode();

    return EXIT_SUCCESS;
}
