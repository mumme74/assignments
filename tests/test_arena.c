#include <stdlib.h>
#include "testrunner.h"

#include "arena.h"

TEST_SETUP(arena_suite)

static struct mem_arena *arena = NULL;

TEST_SETUP_FN(arena_suite)
{
    arena = (struct mem_arena*)malloc(sizeof(struct mem_arena));
}

TEST_TEARDOWN_FN(arena_suite)
{
    free(arena);
    arena = NULL;
}

TEST(arena_suite, init, "Should initialize") {
    mem_arena_init(arena);

    //expectEQ((void*)arena->root, NULL);
}

