#ifndef _ARENA_H_
#define _ARENA_H_

#include <stdint.h>
#include <stddef.h>

#define MEM_ARENA_SEGMENT_DEFAULT_SZ 1024


/**
 * A arena segment
 */
struct mem_arena_segment
{
    struct mem_arena_segment *next; ///< next segment, when we can't alloc enough
    uint8_t *bytes; ///< The memory array ptr
    size_t size; ///< The Size of this arena segment
    uint16_t alloc_idx; ///< the allocate pointer
};

/**
 * The actual memory arena
 */
struct mem_arena
{
    struct mem_arena_segment *root; ///< The first arena segment

};

/**
 * initialize an arena
 *
 * @param arena The arena to initialize
 * @return true if success
 */
void mem_arena_init(struct mem_arena *arena);

/**
 * Allocate new memory from the arena
 *
 * @param arena The arena to associate this allocation with
 * @param size The size to allocate
 * @return void ptr to memory position or NULL if failed
 */
void* mem_arena_alloc(struct mem_arena *arena, size_t size);

/**
 * Free an arenas segment, the actual arena is NOT freed
 */
void mem_arena_free(struct mem_arena* arena);


#endif // _ARENA_H_
