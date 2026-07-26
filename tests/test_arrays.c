#include <string.h>

#include "testrunner.h"
#include "arena.h"

static mem_Arena arena;


// create a char array
#define NAME CharArr
#define T char
#define ARRAY_IMPLEMENTATION
#include "array.template.h"

static CharArr char_arr;

TEST_SETUP(char_arr_suite)

TEST_SUITE_SETUP_FN(char_arr_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(char_arr_suite)
{
    mem_arena_free(&arena);
}

TEST_SETUP_FN(char_arr_suite)
{
    CharArr_init(&char_arr, &arena);
}

TEST(char_arr_suite, push_pop_back, "Should push and pop at back")
{
    expectTrue(CharArr_push_back(&char_arr, 'a'));
    expectTrue(CharArr_push_back(&char_arr, 'b'));
    expectEQ(char_arr.size, 2);
    expectTrue(CharArr_push_back(&char_arr, 'c'));
    expectTrue(CharArr_push_back(&char_arr, 'd'));
    expectEQ(char_arr.elements[0], 'a');
    expectEQ(char_arr.elements[1], 'b');
    expectEQ(char_arr.elements[2], 'c');
    expectEQ(char_arr.elements[3], 'd');
    expectEQ(char_arr.size, 4);
}


TEST(char_arr_suite, push_pop_front, "Should push and pop at front")
{
    expectTrue(CharArr_push_front(&char_arr, 'a'));
    expectEQ(char_arr.size, 1);
    expectTrue(CharArr_push_front(&char_arr, 'b'));
    expectTrue(CharArr_push_front(&char_arr, 'c'));
    expectEQ(char_arr.size, 3);
    expectTrue(CharArr_push_front(&char_arr, 'd'));
    expectEQ(char_arr.elements[3], 'a');
    expectEQ(char_arr.elements[2], 'b');
    expectEQ(char_arr.elements[1], 'c');
    expectEQ(char_arr.elements[0], 'd');
    expectEQ(char_arr.size, 4);
}

TEST(char_arr_suite, insert, "Should test insert")
{
    expectTrue(CharArr_insert(&char_arr, 'a', 0));
    expectEQ(char_arr.size, 1);
    expectEQ(char_arr.elements[0], 'a');

    expectTrue(CharArr_insert(&char_arr, 'b', 0));
    expectEQ(char_arr.size, 2);
    expectEQ(char_arr.elements[0], 'b');
    expectEQ(char_arr.elements[1], 'a');

    expectTrue(CharArr_insert(&char_arr, 'c', 2));
    expectEQ(char_arr.size, 3);
    expectEQ(char_arr.elements[0], 'b');
    expectEQ(char_arr.elements[1], 'a');
    expectEQ(char_arr.elements[2], 'c');

    expectFalse(CharArr_insert(&char_arr, 'd', 4));
    expectEQ(char_arr.size, 3);
    expectEQ(char_arr.elements[0], 'b');
    expectEQ(char_arr.elements[1], 'a');
    expectEQ(char_arr.elements[2], 'c');


    expectTrue(CharArr_insert(&char_arr, 'd', -1));
    expectEQ(char_arr.size, 4);
    expectEQ(char_arr.elements[0], 'b');
    expectEQ(char_arr.elements[1], 'a');
    expectEQ(char_arr.elements[2], 'c');
    expectEQ(char_arr.elements[3], 'd');


    expectTrue(CharArr_insert(&char_arr, 'e', 0));
    expectEQ(char_arr.size, 5);
    expectEQ(char_arr.elements[0], 'e');
    expectEQ(char_arr.elements[1], 'b');
    expectEQ(char_arr.elements[2], 'a');
    expectEQ(char_arr.elements[3], 'c');
    expectEQ(char_arr.elements[4], 'd');

    expectTrue(CharArr_insert(&char_arr, 'f', -2));
    expectEQ(char_arr.size, 6);
    expectEQ(char_arr.elements[0], 'e');
    expectEQ(char_arr.elements[1], 'b');
    expectEQ(char_arr.elements[2], 'a');
    expectEQ(char_arr.elements[3], 'c');
    expectEQ(char_arr.elements[4], 'f');
    expectEQ(char_arr.elements[5], 'd');
}

TEST(char_arr_suite, remove, "Test remove")
{
    expectFalse(CharArr_remove(&char_arr, 0));

    expectTrue(CharArr_push_back(&char_arr, 'a'));
    expectTrue(CharArr_push_back(&char_arr, 'b'));
    expectTrue(CharArr_push_back(&char_arr, 'c'));
    expectTrue(CharArr_push_back(&char_arr, 'd'));

    expectFalse(CharArr_remove(&char_arr, 4));
    expectFalse(CharArr_remove(&char_arr, -1));

    expectTrue(CharArr_remove(&char_arr, 1));
    expectEQ(char_arr.elements[1], 'c');
    expectEQ(char_arr.size, 3);

    expectTrue(CharArr_remove(&char_arr, 2));
    expectEQ(char_arr.elements[1], 'c');
    expectEQ(char_arr.size, 2);

    expectTrue(CharArr_remove(&char_arr, 0));
    expectEQ(char_arr.elements[0], 'c');
    expectEQ(char_arr.size, 1);

    expectTrue(CharArr_remove(&char_arr, 0));
    expectEQ(char_arr.size, 0);

    expectFalse(CharArr_remove(&char_arr, 0));
}

TEST(char_arr_suite, pop_back, "Test pop_back")
{
    expectEQ(CharArr_pop_back(&char_arr), 0);

    expectTrue(CharArr_push_back(&char_arr, 'a'));
    expectTrue(CharArr_push_back(&char_arr, 'b'));
    expectTrue(CharArr_push_back(&char_arr, 'c'));
    expectTrue(CharArr_push_back(&char_arr, 'd'));

    expectEQ(CharArr_pop_back(&char_arr), 'd');
    expectEQ(CharArr_pop_back(&char_arr), 'c');
    expectEQ(CharArr_pop_back(&char_arr), 'b');
    expectEQ(CharArr_pop_back(&char_arr), 'a');
    expectEQ(CharArr_pop_back(&char_arr), 0);
}


TEST(char_arr_suite, pop_front, "Test pop_front")
{
    expectEQ(CharArr_pop_back(&char_arr), 0);

    expectTrue(CharArr_push_back(&char_arr, 'a'));
    expectTrue(CharArr_push_back(&char_arr, 'b'));
    expectTrue(CharArr_push_back(&char_arr, 'c'));
    expectTrue(CharArr_push_back(&char_arr, 'd'));

    expectEQ(CharArr_pop_front(&char_arr), 'a');
    expectEQ(CharArr_pop_front(&char_arr), 'b');
    expectEQ(CharArr_pop_front(&char_arr), 'c');
    expectEQ(CharArr_pop_front(&char_arr), 'd');
    expectEQ(CharArr_pop_front(&char_arr), 0);
}

TEST(char_arr_suite, slice, "Test slices")
{
    CharArrSlice slice = CharArr_slice(&char_arr, 1, 1);
    expectEQ((void*)slice.arr, NULL);
    expectEQ(slice.size, 0);
    expectEQ((void*)slice.elements, NULL);

    expectTrue(CharArr_push_back(&char_arr, 'a'));
    expectTrue(CharArr_push_back(&char_arr, 'b'));
    expectTrue(CharArr_push_back(&char_arr, 'c'));
    expectTrue(CharArr_push_back(&char_arr, 'd'));

    slice = CharArr_slice(&char_arr, 1, 2);
    expectEQ((void*)slice.arr, &char_arr);
    expectEQ(slice.size, 2);
    expectEQ(slice.elements[0], 'b');
    expectEQ(slice.elements[1], 'c');
}

TEST(char_arr_suite, concat, "Test Concat")
{
    CharArr_push_back(&char_arr, 'a');
    CharArr_push_back(&char_arr, 'b');
    CharArr_push_back(&char_arr, 'c');
    CharArr ch_arr2 = {0};
    CharArr_init(&ch_arr2, &arena);
    CharArr_push_back(&ch_arr2, '1');
    CharArr_push_back(&ch_arr2, '2');
    CharArr_push_back(&ch_arr2, '3');
    CharArr_push_back(&ch_arr2, '4');

    expectTrue(CharArr_concat(&char_arr, &ch_arr2));
    expectEQ(char_arr.size, 7);
    expectEQ((const char*)char_arr.elements, "abc1234");

    // trigger growth
    expectTrue(CharArr_concat(&char_arr, &ch_arr2));
    expectEQ(char_arr.size, 11);
    expectEQ((const char*)char_arr.elements, "abc12341234");
    expectGTE(char_arr.capacity, 11);
}

TEST(char_arr_suite, at, "Test at")
{
    CharArr_push_back(&char_arr, 'a');
    CharArr_push_back(&char_arr, 'b');
    CharArr_push_back(&char_arr, 'c');
    expectEQ(CharArr_at(&char_arr, 0), 'a');
    expectEQ(CharArr_at(&char_arr, 1), 'b');
    expectEQ(CharArr_at(&char_arr, 2), 'c');
    expectEQ(CharArr_at(&char_arr, 3), '\0');
}

TEST(char_arr_suite, clear_arr, "Test clear")
{
    CharArr_push_back(&char_arr, 'a');
    CharArr_push_back(&char_arr, 'b');
    CharArr_push_back(&char_arr, 'c');
    CharArr_clear(&char_arr);
    expectEQ(char_arr.size, 0);
    expectEQ(char_arr.elements[0], '\0');
}

//------------------------------------------------

#define NAME StrArr
#define T char*
#define ARRAY_IMPLEMENTATION
#include "array.template.h"


TEST_SETUP(str_arr_suite)

TEST_SUITE_SETUP_FN(str_arr_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(str_arr_suite)
{
    mem_arena_free(&arena);
}

static StrArr str_arr;

TEST_SETUP_FN(str_arr_suite)
{
    StrArr_init(&str_arr, &arena);
}

TEST(str_arr_suite, index_of, "Should scan in str")
{
    expectTrue(StrArr_push_back(&str_arr, "First"));
    expectTrue(StrArr_push_back(&str_arr, "Second"));
    expectTrue(StrArr_push_back(&str_arr, "Third"));
    expectTrue(StrArr_push_back(&str_arr, "Forth"));

    expectEQ(StrArr_index_of(&str_arr, "Second"), 1);
    expectEQ(StrArr_last_index_of(&str_arr, "Second"), 1);
}

// --------------------------------------------------------------

typedef struct TestObj {
    uint8_t member;
    const char* str;
} TestObj;

#define NAME TestObjArr
#define T TestObj
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a, b) \
    a.member == b.member && strcmp(a.str, b.str) == 0
#include "array.template.h"


TEST_SETUP(obj_arr_suite)

TEST_SUITE_SETUP_FN(obj_arr_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(obj_arr_suite)
{
    mem_arena_free(&arena);
}

static TestObjArr obj_arr;

TEST_SETUP_FN(obj_arr_suite)
{
    TestObjArr_init(&obj_arr, &arena);
}

TEST(obj_arr_suite, obj_index_of, "Test index of we objs")
{
    TestObj obj = {10, "TestObj1"};
    expectEQ(TestObjArr_index_of(&obj_arr, obj), -1);
    TestObjArr_push_back(&obj_arr, obj);
    expectEQ(TestObjArr_index_of(&obj_arr, obj), 0);
}