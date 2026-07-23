#include <stdio.h>
#include <stdlib.h>
#include "arena.h"

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    struct mem_arena testdata_arena;
    mem_arena_init(&testdata_arena);


    return EXIT_SUCCESS;
}
