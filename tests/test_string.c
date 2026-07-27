#include <string.h>

#include "testrunner.h"
#include "typestring.h"

TEST_SETUP(str_suite)

static String str;
static mem_Arena arena;

TEST_SUITE_SETUP_FN(str_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(str_suite)
{
    mem_arena_free(&arena);
}

TEST_SETUP_FN(str_suite)
{
    String_init(&str, &arena);
}

TEST(str_suite, str_set, "Should set string")
{
    char buf[] = "Set this string";
    size_t sz = strlen(buf);
    expectTrue(String_set(&str, buf, sz));
    expectEQ(str.size, sz);
    expectEQ(str.elements, buf);

    char buf2[] = "Test 2";
    size_t sz2 = strlen(buf2);
    expectTrue(String_set(&str, buf2, sz2));
    expectEQ(str.size, sz2);
    expectEQ(str.elements, buf2);
}

TEST(str_suite, str_append, "Should append cstr")
{
    char buf[] = "Append this string";
    size_t size = strlen(buf);
    bool res = String_append_str(&str, buf, size);
    expectTrue(res);

    expectEQ(str.size, size);
    expectEQ(str.elements, buf);

    expectTrue(res);
    char buf2[200] = {0};
    strncat(buf2, buf, size);
    strncat(buf2, buf, size);
    res = String_append_str(&str, buf2, size);
    expectEQ(str.elements, buf2);
    expectEQ(str.size, size*2);
}

TEST(str_suite, str_scramble, "Should scramble")
{

    const char buf[] = "abcdefFEDCBA";
    const char expect[] = {
        (char)0xE7, (char)0xAB, (char)0x6E,
        (char)0x2D, (char)0xE3, (char)0xB7,
        (char)0x0B, (char)0xCC, (char)0x80,
        (char)0x4A, (char)0x0F, (char)0xC0,
        0
    };
    size_t len = strlen(buf);

    String scramble = {.arena=&arena};

    String_append_str(&str, buf, len);

    String_scramble(&scramble, &str, 0x2468ACE1);
    expectEQ(scramble.elements, expect);
}

TEST(str_suite, str_unscramble, "Should unscramble")
{

    const char expect[] = "abcdefFEDCBA";
    const char buf[] = {
        (char)0xE7, (char)0xAB, (char)0x6E,
        (char)0x2D, (char)0xE3, (char)0xB7,
        (char)0x0B, (char)0xCC, (char)0x80,
        (char)0x4A, (char)0x0F, (char)0xC0,
        0
    };
    size_t len = strlen(buf);

    String unscramble = {.arena=&arena};

    String_append_str(&str, buf, len);

    String_unscramble(&unscramble, &str, 0x2468ACE1);
    expectEQ(unscramble.elements, expect);
}

TEST(str_suite, utf8_len, "Test print utf 8 len")
{
    const char buf[] = "öäåÅÄÖ";
    String_set(&str, buf, strlen(buf));

    expectEQ(String_utf8_len(&str), 6);
}

TEST(str_suite, utf8_slice, "Test utf8 slice")
{
    const char buf[] = "öäåÅÄÖ";
    String_set(&str, buf, strlen(buf));

    StringSlice sl = String_uft8_slice(&str, 2, 3);
    expectEQ(sl.size, 6);

    String str2;
    String_init(&str2, &arena);
    String_set_from_slice(&str2, &sl);
    expectEQ(str2.elements, "åÅÄ");
}

TEST(str_suite, str_append_slice, "Should append with slice")
{
    const char buf[] = "Teststring";
    String_set(&str, buf, strlen(buf));

    StringSlice sl = String_slice(&str, 2, 5);

    String_append_from_slice(&str, &sl);
    expectEQ(str.elements, "Teststringststr");
    expectEQ(str.size, strlen(buf) + 5);
}


TEST(str_suite, str_set_from_slice, "Should set with slice")
{
    const char buf[] = "Teststring";
    String_set(&str, buf, strlen(buf));

    StringSlice sl = String_slice(&str, 2, 5);

    String str2;
    String_init(&str2, &arena);
    String_set(&str2, buf, strlen(buf));
    String_set_from_slice(&str2, &sl);
    expectEQ(str2.elements, "ststr");
    expectEQ(str2.size, 5);
}


TEST(str_suite, str_split_join, "Tests split and join")
{
    const char buf[] = "row1\r\nrow2\r\nrow3";
    String_set(&str, buf, strlen(buf));
    mem_Arena arena2;
    mem_arena_init(&arena2);

    StringArr *arr = String_split(&str, "\r\n", &arena2);
    expectEQ((void*)arr->arena, &arena2);
    expectEQ(arr->size, 3);
    expectEQ(arr->elements[0].elements, "row1");
    expectEQ(arr->elements[1].elements, "row2");
    expectEQ(arr->elements[2].elements, "row3");

    String *s1 = StringArr_join(arr, NULL, &arena2);
    expectEQ((void*)s1->arena, &arena2);
    expectEQ(s1->elements, "row1row2row3");

    String *s2 = StringArr_join(arr, ";", &arena2);
    expectEQ((void*)s2->arena, &arena2);
    expectEQ(s2->elements, "row1;row2;row3");


    mem_arena_free(&arena2);
}

TEST(str_suite, split_no_match, "Should return 1 string")
{
    const char buf[] = "oneline";
    String_set(&str, buf, strlen(buf));
    mem_Arena arena2;
    mem_arena_init(&arena2);

    StringArr *arr = String_split(&str, "\n", &arena2);
    expectEQ((void*)arr->arena, &arena2);
    expectEQ(arr->size, 1);
    expectEQ(arr->elements[0].elements, buf);

    String *s1 = StringArr_join(arr, NULL, &arena2);
    expectEQ((void*)s1->arena, &arena2);
    expectEQ(s1->elements, buf);

    mem_arena_free(&arena2);
}
