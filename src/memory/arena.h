#ifndef _ARENA_H_
#define _ARENA_H_

#include <stdint.h>
#include <stddef.h>

#define MEM_ARENA_SEGMENT_DEFAULT_SZ 1024


/**
 * A arena segment
 */
typedef struct _mem_arena_Segment
{
    struct _mem_arena_Segment *next; ///< next segment, when we can't alloc enough
    uint8_t *bytes; ///< The memory array ptr
    uint32_t size; ///< The Size of this arena segment
    uint16_t alloc_idx; ///< the allocate pointer
} mem_arena_Segment;

/**
 * The actual memory arena
 */
typedef struct
{
    mem_arena_Segment *root; ///< The first arena segment

} mem_Arena;

/**
 * initialize an arena
 *
 * @param arena The arena to initialize
 * @return true if success
 */
void mem_arena_init(mem_Arena *arena);

/**
 * Allocate new memory from the arena
 *
 * @param arena The arena to associate this allocation with
 * @param size The size to allocate
 * @return void ptr to memory position or NULL if failed
 */
void* mem_arena_alloc(mem_Arena *arena, uint32_t size);

/**
 * Free an arenas segment, the actual arena is NOT freed
 */
void mem_arena_free(mem_Arena* arena);


#endif // _ARENA_H_
