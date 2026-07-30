#include "arena.h"
#include "typestring.h"

#define NO_TEMPLATE_TEST_ATOMS
#include "testatom.h"

#define T TestAtom
#define NAME TestAtomArr
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) \
    a.type == b.type && \
    a.string.size == b.string.size && \
    strncmp(a.string.elements, b.string.elements, a.string.size) == 0
#include "array.template.h"

const char* test_atom_type_to_str(enum TestAtomType type)
{
    switch (type) {
    case Argv_Type:      return "Argv_Type";
    case Stdin_Type:     return "Stdin_Type";
    case Stdout_Type:    return "Stdout_Type";
    default: return NULL;
    }
}

enum TestAtomType test_atom_str_type(const char* str)
{
    if (strcmp(str, "Argv_Type") == 0)
        return Argv_Type;
    else if (strcmp(str, "Stdin_Type") == 0)
        return Stdin_Type;
    else if (strcmp(str, "Stdout_Type") == 0)
        return Stdout_Type;
    return Argv_Type;
}


void TestAtom_init(TestAtom *atom, mem_Arena* arena)
{
    atom->type = 0;
    String_init(&atom->string, arena);
}

StringArr* TestAtom_types_unset(TestAtom* atom, mem_Arena* arena)
{
    StringArr* arr = StringArr_new(arena);
    for (size_t i = 0; i < _TestAtomType_EndMarker; ++i) {
        if (atom->type != i) {
            const char* type = test_atom_type_to_str(i);
            StringArr_append(arr, type, -1);
        }
    }

    return arr;
}