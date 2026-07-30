#ifndef _TESTS_H_
#define _TESTS_H_

#include <stdint.h>
#include "typestring.h"
#include "testatom.h"

/**
 * Controlflags for a test object
 */
enum TestFlags {
    TestFlagUndefined,
    SilentlyIgnoreFails  = 0,
    AbortFirstError      = 0x0001,
    AbortAfterFiveErrors = 0x0002,
    AbortAfterTenErrors  = 0x0004,
    TimeoutTwoSecs       = 0x0008,
    TimeoutTenSecs       = 0x0010,
    _TestFlagsEndMarker
};

/**
 * A specific test session
 */
typedef struct Test {
    enum TestFlags flags; ///< flags for this test
    String identifier; ///< A identifier for this test
    String command; ///< The command to run on the client
    TestAtomArr atoms; ///< The test objects for this test
} Test;

/**
 * Return string representation of flags
 */
const char* test_flag_to_str(enum TestFlags flag);

/**
 * Return flag from a mathing string
 */
enum TestFlags test_flag_str_to_flag(const char* str);

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

/**
 * Returns all selected flags as a string array
 */
StringArr* Test_flags(Test* test, mem_Arena* arena);

/**
 * Returns all the free unselected flags as string array
 */
StringArr* Test_flags_unset(Test* test, mem_Arena* arena);

/**
 * Checks if flag is set
 */
bool Test_check_flag(Test* test, enum TestFlags flag);

/**
 * Sets flag
 * @return true if it was previously cleared
 */
bool Test_set_flag(Test* test, enum TestFlags flag);

/**
 * Clears a flag
 * @return true if it was previously set
 */
bool Test_clear_flag(Test* test, enum TestFlags flag);



#endif // _TESTS_H_
