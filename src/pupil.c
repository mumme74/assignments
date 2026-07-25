#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    ui_printf("--- ANSI TUI (Press 'q' to quit) ---\n");
    for (int i = 0; i < 10; i++) {
        ui_printf("|                                  |\n");
    }
    ui_printf("------------------------------------\n");

    ui_render();
}

bool ui_frame()
{
    // 3. Move cursor and draw interactive element
    // \x1b[%d;%dH moves cursor to line Y, column X
    //printf("\x1b[%d;%dH\x1b[1;32m@\x1b[0m", y, x);
    //fflush(stdout);

    char c;

    // 4. Read input
    if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 'q') return false;

        // Handle WASD movement keys
        if (c == 'w') ui_move_cursor_vert(-1);
        if (c == 's') ui_move_cursor_vert(1);
        if (c == 'a') ui_move_cursor_horz(-1);
        if (c == 'd') ui_move_cursor_horz(1);
        if (c == 'p') {printf("print"); fflush(stdout);}
    }

    return true;
}

void run_ui(Document *doc)
{
    ui_enable_raw_mode();
    ui_set_cursor_show(true);

    bool contin = true;
    while (contin) {
        ui_redraw(doc);
        contin = ui_frame(doc);
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
