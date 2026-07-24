#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"


mem_arena_Segment* alloc_segment(size_t size) {
    // grow in multiples of...
    size_t sz = MEM_ARENA_SEGMENT_DEFAULT_SZ;
    while (sz < size)
        sz += MEM_ARENA_SEGMENT_DEFAULT_SZ;

    mem_arena_Segment *seg = (mem_arena_Segment*)
        calloc(1, sizeof(mem_arena_Segment));

    if (!seg) return NULL;

    seg->bytes = (uint8_t*)calloc(1, sz);
    if (!seg->bytes) return NULL;

    seg->size = sz;

    return seg;
}

// ----------------------------------------------


void mem_arena_init(mem_Arena *arena)
{
    arena->root = NULL;
}

void* mem_arena_alloc(mem_Arena *arena, size_t size)
{
    assert(arena != NULL);

    // initial alloc
    if (arena->root == NULL) {
        arena->root = alloc_segment(size);
        if (!arena->root)
            return NULL;
    }

    // find a big enough arena
    mem_arena_Segment *seg = arena->root,
                      *prev = NULL;
    for (; seg != NULL; seg = seg->next) {
        if (seg->size - seg->alloc_idx >= size)
            break;
        prev = seg;
    }

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

void* mem_arena_realloc(
    mem_Arena* arena, uint8_t* data, size_t prevSize, size_t newSize
) {
    // find segment
    mem_arena_Segment *seg = arena->root;
    for (; seg != NULL; seg = seg->next) {
        if (seg->bytes <= data && &seg->bytes[seg->size-1] >= data)
            break;
    }

    // we can just append the new size
    if (seg &&
        data + prevSize == &seg->bytes[seg->alloc_idx] &&
        seg->size-seg->alloc_idx >newSize
    ) {
        seg->alloc_idx += newSize - prevSize;
        return data;
    }

    // need to allocate new and move it
    uint8_t *newData = mem_arena_alloc(arena, newSize);
    if (!newData) return NULL;

    memcpy(newData, data, prevSize);

    // shrink this one
    if (seg && data + prevSize == &seg->bytes[seg->alloc_idx])
        seg->alloc_idx -= prevSize;

    return newData;
}

void mem_arena_free(mem_Arena* arena) {
    for (mem_arena_Segment *seg = arena->root, *next = NULL;
         seg != NULL; seg = next)
    {
        next = seg->next;
        free(seg->bytes);
        free(seg);
    }
}
