#include <stdio.h>
#include <stdlib.h>
#include "arena.h"

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    mem_Arena testdata_arena;
    mem_arena_init(&testdata_arena);


    return EXIT_SUCCESS;
}
