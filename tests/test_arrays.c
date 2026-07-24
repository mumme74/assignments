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


