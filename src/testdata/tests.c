#include "arena.h"

#define NO_TEMPLATE_TESTS
#include "tests.h"

#define T Test
#define NAME TestArr
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) \
    strcmp(a.identifier.elements, b.identifier.elements) == 0 && \
    strcmp(a.command.elements, b.command.elements) == 0 && \
    a.atoms.size == b.atoms.size
#include "array.template.h"


// ------------------------------------------------

void Test_init(Test *test, mem_Arena* arena)
{
    String_init(&test->identifier, arena);
    String_init(&test->command, arena);
    TestAtomArr_init(&test->atoms, arena);
}
