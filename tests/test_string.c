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
