#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "arena.h"
#include "document.h"
#include "terminal.h"
#include "utils.h"
#include "controls.h"

Document* read_doc(const char* filepath)
{
    mem_Arena *arena = (mem_Arena*)malloc(sizeof(mem_Arena));
    mem_arena_init(arena);

    Document *doc = (Document*)malloc(sizeof(Document));
    Document_init(doc, arena);

    FILE* fp = fopen(filepath, "r");
    if (fp && Document_read(doc, fp))
        return doc; // caller takes ownership of arena

    mem_arena_free(arena);
    free(doc);
    free(arena);

    return NULL;
}
/*
int start = 0, cur_row = 0;
void ui_redraw(Document* doc)
{
    (void)doc;
    // Draw UI boundaries
    int rows, cols, i;
    ui_get_screen_size(&cols, &rows);

    // header
    ui_one_format(Format.ResetAll);
    ui_one_format(FrontColors.White);
    ui_one_format(BackColors.LightBlue);
    ui_printf("--- ANSI TUI (Press 'q' to quit) ---\n");
    ui_one_format(Format.ResetAll);
    //ui_set_scrollable_rows(1, cols);

    int end = start + rows - 2;
    for (i = start; i < (MIN(end, 100)); i++) {
        ui_printf("|                    %d              |\n", i);
    }
    ui_printf("------------------------------------");

    ui_render();
}

void scroll(int c) {
    int cols, rows, x, y;
    ui_get_screen_size(&cols, &rows);
    ui_get_cursor_pos(&x, &y);

    switch (c) {
    case Key_ArrowLeft: ui_move_cursor_horz(-1); break;
    case Key_ArrowRight: ui_move_cursor_horz(1); break;
    case Key_ArrowUp:
        cur_row = MAX(0, cur_row-1);
        if (cur_row < start)
            start--;
        if (y > 2)
            ui_move_cursor_vert(-1);
        break;
    case Key_ArrowDown:
        cur_row = MIN(100, cur_row+1);
        if (cur_row > start + rows -3)
            start++;
        if (y < rows)
            ui_move_cursor_vert(1);
        break;
    default: break;
    }
}
*/

static void get_list_row(ui_Wrapper* wrap, String* text, int row)
{
    (void)wrap;
    const char buf[] = "Detta är en extra lång list sträng!!!";
    char buf2[100] = {0};
    sprintf(buf2, "%d %s", row, buf);
    String_set(text, buf2, strlen(buf2));
}



static ui_Window* create_page1(mem_Arena* arena)
{
    ui_Window *win = (ui_Window*)mem_arena_alloc(arena, sizeof(ui_Window));
    if (!win) return NULL;

    ui_window_init(win, arena);

    ui_Wrapper *hdr = ui_window_new_control(win, UI_ContainerType),
               *menu_btn1 = ui_window_new_control(win, UI_ButtonType),
               *menu_btn2 = ui_window_new_control(win, UI_ButtonType),
               *list1 = ui_window_new_control(win, UI_ListType),
               *text_edit = ui_window_new_control(win, UI_TextEditType),
               *footer = ui_window_new_control(win, UI_ContainerType),
               *lbl1  = ui_window_new_control(win, UI_LabelType);

    list1->list->fetch_row = get_list_row;

    int rows, cols;
    ui_get_screen_size(&cols, &rows);

    ui_rect_set(&hdr->rect, 0,0, cols, 2);
    ui_rect_set(&menu_btn1->rect, 4,1, 10,4);
    ui_rect_set(&menu_btn2->rect, 20,2, 30,2);
    ui_rect_set(&list1->rect, 5,5, 30,10);
    ui_rect_set(&footer->rect, 0,rows, cols,rows);
    ui_rect_set(&text_edit->rect, 5,12, 20,16);
    ui_rect_set(&lbl1->rect, 3,rows-1, 20,rows-1);
    menu_btn1->button->horz_align = HorzAlignCenter;
    menu_btn2->button->horz_align = HorzAlignRight;
    menu_btn1->textedit->vert_align = VertAlignCenter;

    ui_window_append(win, hdr);
    ui_window_append(win, list1);
    ui_window_append(win, text_edit);
    ui_window_append(win, footer);
    ui_control_append(hdr, menu_btn1);
    ui_control_append(hdr, menu_btn2);
    ui_control_append(footer, lbl1);

    String_set(&menu_btn1->button->text, "Help öäå", 4);
    String_set(&menu_btn2->button->text, "Save", 4);
    String_set(&text_edit->textedit->text, "Textedit", 8);
    String_set(&lbl1->label->text, "Bottomlbl", 9);

    return win;
}

static int page_nr = 0;

ui_Window* ui_repaint(Document* doc, mem_Arena* arena)
{
    (void)doc;
    mem_arena_free(arena);
    mem_arena_init(arena);
    if (page_nr == 0)
        return create_page1(arena);

    return NULL;
}


void run_ui(Document *doc)
{
    ui_enable_raw_mode();
    ui_set_cursor_show(true);

    mem_Arena arena;
    mem_arena_init(&arena);

    ui_Window *win = ui_repaint(doc, &arena);

    bool contin = true,
         update_ui = true;

    while (contin) {
        int c;
        if (update_ui) {
            update_ui = false;
        }


        if ((c = toupper(ui_window_listen(win))) > 0) {
            //scroll(c);
            //update_ui = true;
            // change page here
            switch (c) {
            //case Key_Tab: ui_window_nav_forward(win); break;
            //case 'R': ui_repaint(doc, &arena); break;
            //case 'Q': contin = false; break;
            //case 'P': printf("print"); fflush(stdout); break;
            default: break;
                //printf("unhandled: %d  %c\n", c, c);
            }
        }
    }

    mem_arena_free(&arena);

    ui_disable_raw_mode();
}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    Document*doc = read_doc("../tests/testdoc");
    if (!doc)
        return EXIT_FAILURE;

    run_ui(doc);

    mem_arena_free(doc->persons.arena);
    free(doc->persons.arena);
    free(doc);

    return EXIT_SUCCESS;
}
