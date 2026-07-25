#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "arena.h"
#include "document.h"
#include "terminal.h"

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

void ui_redraw(Document* doc)
{
    (void)doc;
    // Draw UI boundaries
    ui_one_command(Format.ResetAll);
    ui_one_command(FrontColors.Blue);
    ui_one_command(BackColors.LightYellow);
    ui_printf("--- ANSI TUI (Press 'q' to quit) ---\n");
    for (int i = 0; i < 10; i++) {
        ui_printf("|                                  |\n");
    }
    ui_printf("------------------------------------\n");

    ui_render();
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
            switch (c) {
            case 'Q': contin = false; break;
            case Key_ArrowDown: ui_move_cursor_vert(1); break;
            case Key_ArrowUp: ui_move_cursor_vert(-1); break;
            case Key_ArrowLeft: ui_move_cursor_horz(-1); break;
            case Key_ArrowRight: ui_move_cursor_horz(1); break;
            case 'P': printf("print"); fflush(stdout); break;
            default:
                printf("unhandled: %d  %c\n", c, c);
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
