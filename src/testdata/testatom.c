#include "arena.h"
#include "typestring.h"

#define NO_TEMPLATE_TEST_ATOMS
#include "testatom.h"

#define T TestAtom
#define NAME TestAtomArr
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) \
    a.type == b.type && \
    a.flags == b.flags && \
    a.string.size == b.string.size && \
    strncmp(a.string.elements, b.string.elements, a.string.size) == 0
#include "array.template.h"



void TestAtom_init(TestAtom *atom, mem_Arena* arena)
{
    atom->type = 0;
    atom->flags = 0;
    String_init(&atom->string, arena);
}
