#ifndef _TESTS_H_
#define _TESTS_H_

#include <stdint.h>
#include "typestring.h"
#include "testatom.h"

/**
 * Controlflags for a test object
 */
enum TestFlags {
    SilentlyIgnoreFails = 0,
    AbortFirstError = 0x0001,
    AbortAfterFiveErrors = 0x0002,
    AbortAfterTenErrors = 0x0004,
};

/**
 * A specific test session
 */
typedef struct Test {
    String identifier; ///< A identifier for this test
    String command; ///< The command to run on the client
    TestAtomArr atoms; ///< The test objects for this test
} Test;

/**
 * An array of test sessions
 */
#ifndef NO_TEMPLATE_TESTS

#define T Test
#define NAME TestArr
#include "array.template.h"

#endif


// --------------------------------------------------------------



void Test_init(Test *test, mem_Arena* arena);



#endif // _TESTS_H_
