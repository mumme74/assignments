#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "arena.h"
#include "document.h"
#include "terminal.h"
#include "utils.h"

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

void run_ui(Document *doc)
{
    ui_enable_raw_mode();
    ui_set_cursor_show(true);

    bool contin = true,
         update_ui = true;

    while (contin) {
        int c;
        if (update_ui) {
            update_ui = false;
            ui_redraw(doc);
        }

        if ((c = toupper(ui_listen())) > 0) {
            scroll(c);
            update_ui = true;

            switch (c) {
            case 'Q': contin = false; break;
            case 'P': printf("print"); fflush(stdout); break;
            default: break;
                //printf("unhandled: %d  %c\n", c, c);
            }
        }
    }

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
