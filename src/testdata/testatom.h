#ifndef _TEST_ATOM_H_
#define _TEST_ATOM_H_

#include "typestring.h"

/**
 * The type of object
 */
enum TestAtomType {
    Unhandled_Type,
    Argv_Type,
    Stdin_Type,
    Stdout_Type,
};

/**
 * A atomic thing to do or expect during a test session
 */
typedef struct TestAtom{
    enum TestAtomType type; ///< The type of test to perform
    String string; ///< the string to send or expect
    uint16_t flags;  ///< Controlling flags for this test
} TestAtom;


#ifndef NO_TEMPLATE_TEST_ATOMS

#define T TestAtom
#define NAME TestAtomArr
#include "array.template.h"

#endif


void TestAtom_init(TestAtom *atom, mem_Arena* arena);


#endif // _TEST_ATOM_H_

