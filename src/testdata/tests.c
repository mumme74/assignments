#include "arena.h"

#define NO_TEMPLATE_TESTS
#include "tests.h"

#define T Test
#define NAME TestArr
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) \
    a.identifier.size == b.identifier.size && \
    strncmp(a.identifier.elements, b.identifier.elements, a.identifier.size) == 0 && \
    a.command.size == b.command.size && \
    strncmp(a.command.elements, b.command.elements, a.command.size) == 0 && \
    a.atoms.size == b.atoms.size
#include "array.template.h"


const char* test_flag_to_str(enum TestFlags flag)
{
    switch (flag) {
    case SilentlyIgnoreFails:  return "SilentlyIgnoreFails";
    case AbortFirstError:      return "AbortFirstError";
    case AbortAfterFiveErrors: return "AbortAfterFiveErrors";
    case AbortAfterTenErrors:  return "AbortAfterTenErrors";
    case TimeoutTwoSecs:       return "TimeoutTwoSecs";
    case TimeoutTenSecs:       return "TimeoutTenSecs";
    default: return NULL;
    }
}

enum TestFlags test_flag_str_to_flag(const char* str)
{
    if ((strcmp(str, "SilentlyIgnoreFails") == 0))
        return SilentlyIgnoreFails;
    else if (strcmp(str, "AbortFirstErroe") == 0)
        return AbortFirstError;
    else if (strcmp(str, "AbortAfterFiveErrors") == 0)
        return AbortAfterFiveErrors;
    else if (strcmp(str, "AbortAfterTenErrors") == 0)
        return AbortAfterTenErrors;
    else if (strcmp(str, "TimeoutTwoSecs") == 0)
        return TimeoutTwoSecs;
    else if (strcmp(str, "TimeoutTenSecs") == 0)
        return TimeoutTenSecs;
    return TestFlagUndefined;
}

// ------------------------------------------------

void Test_init(Test *test, mem_Arena* arena)
{
    String_init(&test->identifier, arena);
    String_init(&test->command, arena);
    TestAtomArr_init(&test->atoms, arena);
}

StringArr* Test_flags(Test* test, mem_Arena* arena)
{
    StringArr *arr = StringArr_new(arena);
    for (size_t i = 0; i < 32; ++i) {
        if ((test->flags & (0x01 << i)) != 0)
            StringArr_append(arr, test_flag_to_str(i), -1);
    }

    return arr;
}
