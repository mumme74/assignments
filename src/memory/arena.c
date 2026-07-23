#include <assert.h>
#include <stdlib.h>
#include "arena.h"


struct mem_arena_segment* alloc_segment(size_t size) {
    // grow in multiples of...
    size_t sz = MEM_ARENA_SEGMENT_DEFAULT_SZ;
    while (sz < size)
        sz += MEM_ARENA_SEGMENT_DEFAULT_SZ;

    struct mem_arena_segment *seg = (struct mem_arena_segment*)
        calloc(1, sizeof(struct mem_arena_segment));

    if (!seg) return NULL;

    seg->bytes = (uint8_t*)malloc(sz);
    if (!seg->bytes) return NULL;

    seg->size = sz;

    return seg;
}

// ----------------------------------------------


void mem_arena_init(struct mem_arena *arena)
{
    arena->root = NULL;
}

void* mem_arena_alloc(struct mem_arena *arena, size_t size)
{
    assert(arena != NULL);

    // initial alloc
    if (arena->root == NULL) {
        arena->root = alloc_segment(size);
        if (!arena->root)
            return NULL;
    }

    // find a big enough arena
    struct mem_arena_segment *seg = arena->root,
                             *prev = NULL;
    while (seg && seg->size-seg->alloc_idx < size)
        prev = seg;

    // grow
    if (!seg) {
        seg = alloc_segment(size);
        if (!seg) return NULL;

        prev->next = seg;
    }

    uint8_t *ptr = &seg->bytes[seg->alloc_idx];
    seg->alloc_idx += size;

    return ptr;
}

void mem_arena_free(struct mem_arena* arena) {
    for (struct mem_arena_segment *seg = arena->root, *next = NULL;
         seg != NULL; seg = next)
    {
        next = seg->next;
        free(seg->bytes);
        free(seg);
    }
}
