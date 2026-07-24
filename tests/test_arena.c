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

    const char testStr[10] = "testing10";
    strcat(ptr, testStr);
    expectEQ((const char*)ptr, testStr);
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

TEST(arena_suite, realloc, "Should realloc")
{
    void* ptr1 = mem_arena_alloc(arena, 100);
    const char testStr[] = "Test if this moves along";
    strcat((char*)ptr1, testStr);

    void* ptr1_shrinked = mem_arena_realloc(arena, ptr1, 100, 30);
    expectEQ((void*)ptr1, ptr1_shrinked);
    expectEQ((const char*)ptr1, testStr);

    void* ptr1_grown = mem_arena_realloc(arena, ptr1, 30, 200);
    expectEQ((void*)ptr1_grown, ptr1);
    expectEQ((const char*)ptr1, testStr);

    void* ptr2 = mem_arena_alloc(arena, 100);
    expectGT((void*)ptr2, ptr1);

    void* ptr2_shrink = mem_arena_realloc(arena, ptr2, 100, 30);
    expectEQ(ptr2_shrink, ptr2);

    void* ptr1_mov_same_seg = mem_arena_realloc(arena, ptr1, 200, 300);
    expectEQ(ptr1_mov_same_seg, (uint8_t*)ptr2 + 30);
    expectEQ((const char *)ptr1_mov_same_seg, testStr);

    void* ptr1_mov_new_seg = mem_arena_realloc(arena, ptr1, 300, 2048);
    expectGT(ptr1_mov_new_seg, ptr1);
    expectEQ((const char*)ptr1_mov_new_seg, testStr);
}


