/**
 * This is a generic array using the template pattern
 *
 * Based on: https://www.innercomputing.com/blog/generic-collections-in-c
 *
 * It should be included like:
 * #define T int                ///< The type of each element
 * #define NAME array_int       ///< The name of the collection
 * #define ARRAY_IMPLEMENTATION ///< Define implementation, once per translation unit
 * #include "array.template.h"  ///< Instanciates this template
 *
 * Optional:
 * #define CHECK_EQUAL(a, b) \   ///< If storing a non comparable object such as a obj
 *          a.prop1 == b.prop1 && a.prop2 == b.prop2
 *
 * #define ARRAY_INIT_SZ 8      ///< The initial size
 * #define ARRAY_GROW_BY 2      ///< The growth factor, 2 doubles each time
 *
 */

#ifndef T
# error Must define type T to use array.template.h
#endif

#ifndef ARRAY_INIT_SZ
#define ARRAY_INIT_SZ 8
#endif

#ifndef ARRAY_GROW_BY
#define ARRAY_GROW_BY 2
#endif

#ifndef CHECK_EQUAL
#define CHECK_EQUAL(a,b) a==b
#endif

// dev mode:squelsh dev error
// comment out in production

//#define T int
//#define NAME DEV_ARR
//#define ARRAY_IMPLEMENTATION

// end dev mode

#include <stddef.h>
#include <string.h> // for memset
#include <stdbool.h>
#include "arena.h"

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)


#define init CAT(NAME, _init)
#define resize CAT(NAME, _resize)
#define push_back CAT(NAME, _push_back)
#define push_front CAT(NAME, _push_front)
#define insert CAT(NAME, _insert)
#define remove CAT(NAME, _remove)
#define pop_back CAT(NAME, _pop_back)
#define pop_front CAT(NAME, _pop_front)
#define index_of CAT(NAME, _index_of)
#define last_index_of CAT(NAME, _last_index_of)
#define slice CAT(NAME, _slice)
#define concat CAT(NAME, _concat)
#define at CAT(NAME, _at)

#define NAME_SLICE CAT(NAME, Slice)


typedef struct NAME {
    size_t size;
    size_t capacity;
    T* elements;
    mem_Arena *arena;
} NAME;

typedef struct NAME_SLICE {
    NAME *arr; ///< The array this slice applies to
    T* elements;
    size_t size;
} NAME_SLICE;

void init(NAME* arr, mem_Arena* arena);
bool resize(NAME* arr, size_t capacity);
bool push_back(NAME* arr, T element);
bool push_front(NAME* arr, T element);
bool insert(NAME* arr, T element, int32_t atIdx);
bool remove(NAME* arr, int32_t idx);
T pop_back(NAME* arr);
T pop_front(NAME* arr);
int32_t index_of(NAME* arr, T element);
int32_t last_index_of(NAME* arr, T element);
NAME_SLICE slice(NAME* arr, size_t start, int32_t len);
bool concat(NAME* dest, NAME* src);
T at(NAME* arr, size_t idx);



#ifdef ARRAY_IMPLEMENTATION

#define GROW_IF_NEEDED(arr) \
    if (arr->size >= arr->capacity &&                              \
        !resize(arr, arr->capacity ? arr->capacity * ARRAY_GROW_BY \
                                   : ARRAY_INIT_SZ)                \
    )                                                              \
        return false /*failed to resize*/

void init(NAME* arr, mem_Arena* arena)
{
    arr->capacity = arr->size = 0;
    arr->arena = arena;
    arr->elements = NULL;
}

bool resize(NAME* arr, size_t capacity)
{
    T* old = arr->elements;
    arr->elements = (T*)mem_arena_realloc(
        arr->arena, (uint8_t*)arr->elements,
        arr->capacity * sizeof(T), capacity * sizeof(T));

    if (!arr->elements) {
        arr->elements = old;
        return false;
    }

    arr->capacity = capacity;

    return true;
}

bool push_back(NAME* arr, T element)
{
    GROW_IF_NEEDED(arr);

    arr->elements[arr->size++] = element;

    return true;
}

bool push_front(NAME* arr, T element)
{
    GROW_IF_NEEDED(arr);

    if (!arr->size)
        return push_back(arr, element);

    for (int j = arr->size, i = arr->size-1; j > 0; --j, --i)
        arr->elements[j] = arr->elements[i];

    arr->elements[0] = element;
    ++arr->size;

    return true;
}

bool insert(NAME* arr, T element, int32_t atIdx)
{
    if (atIdx > (int32_t)arr->size)
        return false;

    if (atIdx < 0)
        return insert(arr, element, arr->size + 1 + atIdx);

    if (arr->size == 0)
        return push_front(arr, element);

    GROW_IF_NEEDED(arr);

    for (int i = arr->size, j = arr->size-1; i > atIdx; --i, --j)
        arr->elements[i] = arr->elements[j];

    arr->elements[atIdx] = element;
    ++arr->size;

    return true;
}

bool remove(NAME* arr, int32_t idx)
{
    if ((int32_t)arr->size <= idx || idx < 0)
        return false;

    for (size_t i = idx+1, j = idx; i < arr->size; ++i, ++j)
        arr->elements[j] = arr->elements[i];

    T empty = {0};
    arr->elements[--arr->size] = empty;

    return true;
}

T pop_back(NAME* arr)
{
    T empty = {0};
    if (arr->size == 0)
        return empty;

    T element = arr->elements[arr->size-1];
    arr->elements[--arr->size] = empty;

    return element;
}

T pop_front(NAME* arr)
{
    T empty = {0};
    if (arr->size == 0)
        return empty;

    T element = arr->elements[0];
    remove(arr, 0);

    return element;
}

int32_t index_of(NAME* arr, T element)
{
    for (int32_t i = 0; i < (int32_t)arr->size; ++i) {
        if (CHECK_EQUAL(arr->elements[i], element))
            return i;
    }

    return -1;
}

int32_t last_index_of(NAME* arr, T element)
{
    for (int32_t i = arr->size-1; i > -1; --i) {
        if (CHECK_EQUAL(arr->elements[i], element))
            return i;
    }

    return -1;
}

NAME_SLICE slice(NAME* arr, size_t start, int32_t len)
{
    NAME_SLICE slice = {0};
    if (arr->size <= start)
        return slice;

    if (len < 0)
        len = arr->size-1;

    slice.arr = arr;
    slice.elements = &arr->elements[start];
    slice.size = (size_t)len;

    return slice;
}

bool concat(NAME* dest, NAME* src)
{
    size_t needed = dest->size + src->size;
    if (needed > dest->capacity &&
        !resize(dest, needed)
    )
        return false;

    memcpy(&dest->elements[dest->size], src->elements, src->size * sizeof(T));

    dest->size += src->size;

    return true;
}

T at(NAME* arr, size_t idx)
{
    if (arr->size > idx)
        return arr->elements[idx];

    T empty = {0};
    return empty;
}


#endif

#undef init
#undef resize
#undef push_back
#undef push_front
#undef insert
#undef remove
#undef pop_back
#undef pop_front
#undef index_of
#undef last_index_of
#undef slice
#undef concat
#undef at

#undef T
#undef NAME

#undef GROW_IF_NEEDED
#undef ARRAY_GROW_BY
#undef ARRAY_INIT_SZ
#undef CHECK_EQUAL
