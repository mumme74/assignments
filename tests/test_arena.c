#include <stdlib.h>
#include "testrunner.h"

#include "arena.h"

TEST_SETUP(arena_suite)

static mem_Arena *arena = NULL;

TEST_SETUP_FN(arena_suite)
{
    arena = (mem_Arena*)malloc(sizeof(mem_Arena));
    mem_arena_init(arena);
}

TEST_TEARDOWN_FN(arena_suite)
{
    if (arena->root)
        mem_arena_free(arena);
    free(arena);
    arena = NULL;
}

TEST(arena_suite, init, "Should initialize")
{
    arena->root = (void*)0x01;
    expectNE((void*)arena->root, NULL);

    mem_arena_init(arena);

    expectEQ((void*)arena->root, NULL);
}

TEST(arena_suite, alloc_10bytes, "Should allocate 10 bytes")
{
    void* ptr = mem_arena_alloc(arena, 10);
    expectNE(ptr, NULL);
    expectNE((void*)arena->root, NULL);
    expectEQ((void*)arena->root->next, NULL);
    expectEQ(arena->root->alloc_idx, 10);
    expectEQ(arena->root->size, MEM_ARENA_SEGMENT_DEFAULT_SZ);
}

TEST(arena_suite, alloc_2segments, "Should allocate 2 segments")
{
    void* ptr1 = mem_arena_alloc(arena, 10);
    expectEQ((void*)arena->root->next, NULL);
    void* ptr2 = mem_arena_alloc(arena, MEM_ARENA_SEGMENT_DEFAULT_SZ*2);
    expectNE((void*)arena->root->next, NULL);

    expectNE((void*)ptr1, NULL);
    expectNE((void*)ptr2, NULL);

    expectEQ(arena->root->alloc_idx, 10);
    expectEQ(arena->root->size, MEM_ARENA_SEGMENT_DEFAULT_SZ);
    expectEQ(arena->root->next->size, MEM_ARENA_SEGMENT_DEFAULT_SZ*2);
    expectEQ(arena->root->next->alloc_idx, MEM_ARENA_SEGMENT_DEFAULT_SZ*2);

    void* ptr3 = mem_arena_alloc(arena, 20);
    uint8_t diff = (uint8_t*)ptr3 - (uint8_t*)ptr1;
    expectNE((void*)ptr3, NULL);
    expectEQ(diff, 10);
}


