#include <string.h>

#include "testrunner.h"
#include "typestring.h"

TEST_SETUP(str_suite)

static types_String str;
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
    types_string_init(&str);
}

TEST(str_suite, str_init, "Should initialize")
{
    types_String str;
    str.data = (void*)0x01;
    types_string_init(&str);

    expectEQ(NULL, str.data);
}

TEST(str_suite, str_prealloc, "Should prealloc")
{
    types_string_pre_alloc(&str, 30, &arena);
    expectNE((void*)str.data, NULL);

    expectEQ(str.size, 31);

    expectEQ(str.data, "");
    expectEQ(str.data[30], 0);
}

TEST(str_suite, str_push, "Should append")
{
    char buf[] = "Append this string";
    size_t len = strlen(buf);
    bool res = types_string_push_str(&str, buf, len, &arena);
    expectTrue(res);

    expectEQ(str.len, len);
    expectEQ(str.data, buf);

    res = types_string_push_str(&str, buf, len, &arena);
    expectTrue(res);
    char buf2[200] = {0};
    strncat(buf2, buf, len);
    strncat(buf2, buf, len);
    expectEQ(str.data, buf2);
    expectEQ(str.len, len*2);
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

    types_String scramble;
    types_string_pre_alloc(&scramble, len, &arena);

    types_string_push_str(&str, buf, len, &arena);

    types_string_scramble(&scramble, &str, 0x2468ACE1);
    expectEQ(scramble.data, expect);
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

    types_String unscramble;
    types_string_pre_alloc(&unscramble, len, &arena);

    types_string_push_str(&str, buf, len, &arena);

    types_string_unscramble(&unscramble, &str, 0x2468ACE1);
    expectEQ(unscramble.data, expect);
}
