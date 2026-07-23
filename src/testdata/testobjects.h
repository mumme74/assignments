#ifndef _TEST_OBJECTS_H_
#define _TEST_OBJECTS_H_

#include <stdint.h>
#include "types.h"

#define TESTDATA_HDR_IDENTIFIER 0xFACEFEED
#define TESTDATA_HDR_VERSION 0x01

/*

Structure of testData
+------------------------------+
| testdata_Header              |
+------------------------------+
| testdata_Persons        |
+------------------------------+
| testdata_Person 1            |
| testdata_Person 2            |
| ...                          |
+------------------------------+
| testdata_Tests               |
+------------------------------+
| testdata_Test 1              |
|     testdata_Obj1 (input)    |
|     testdata_Obj2 (expected) |
| testdata_Test 2              |
|     testdata_Obj1 (input)    |
|     testdata_Obj2 (expected) |
| ...                          |
+------------------------------+

*/


/**
 * The type of object
 */
enum testdata_Type {
    Unhandled_Type,
    Argv_Type,
    Stdin_Type,
    Stdout_Type,
};

/**
 * The roles a person can have
 */
enum testdata_person_Roles {
    Teacher = 0x01,
    Student = 0x02,
    External = 0x04,
    Other    = 0x08
};

/**
 * Controlflags for a test object
 */
enum testdata_test_obj_Flags {
    SilentlyIgnoreFails = 0,
    AbortFirstError = 0x0001,
    AbortAfterFiveErrors = 0x0002,
    AbortAfterTenErrors = 0x0004,
};


/**
 * The header of for the binary format
 */
typedef struct  {
    uint32_t identifier; ///< Identifier for big/little endian
    uint32_t byte_len; ///< The length in bytes of this blob, excluding header
    uint64_t date_compiled; ///< A UTC timestamp when this was compiled
    uint8_t  compiler_person; ///< The person that compiled this blob
    uint8_t  version; ///< The version of the storageformat of this data blob
    uint8_t _pad[5]; // align to 8bytes multiples
} testdata_Header;

/**
 * The header for all persons list
 */
typedef struct {
    uint32_t size; ///< Allocated size
    uint32_t len; ///< Num of persons in this doc
    uint16_t num_persons; ///< How many persons in this list
} testdata_Persons;

/**
 * A single person
 */
typedef struct
{
    uint8_t roles_mask; ///< The testdata_person_Roles this person has
    types_String name; ///< the name of this person.
    types_String email; ///< the email of this person.
} testdata_Person;


/**
 * The actual tests header
 */
typedef struct
{
    uint32_t size; ///< Num bytes in this section
    uint16_t num_tests; ///< How many tests stored
} testdata_Tests;

/**
 * A specific test session
 */
typedef struct {
    types_String identifier; ///< A identifier for this test
    types_String command; ///< The command to run on the client
    uint16_t num_test_objs; ///< The number of testObjects in this test
} testdata_Test;

/**
 * A atomic thing to do or expect during a test session
 */
typedef struct {
    enum testdata_Type type; ///< The type of test to perform
    types_String data; ///< the string to send or expect
    uint16_t flags;  ///< Controlling flags for this test
} testdata_Obj;


/**
 * A complete document in the order as its structured.
 */
typedef struct {
    testdata_Header header;
    testdata_Persons persons_hdr;
    testdata_Person *persons;
    testdata_Tests tests_list_hdr;
    testdata_Test *tests;
} testdata_Document;

// -----------------------------------------------------------------------

void testdata_Header_init(testdata_Header *header);
void testdata_Persons_init(testdata_Persons *persons);
void testdata_Person_init(testdata_Person *person);
void testdata_Tests_init(testdata_Tests *tests);
void testdata_Obj_init(testdata_Obj *obj);
void testdata_Document_init(testdata_Document *doc);

bool testdata_Persons_add();

/**
 * Add a person to the document
 *
 * @param doc The document to add the person to
 */
void testdata_Document_add_person(
    testdata_Document *doc, testdata_Person *person);





#endif // _TEST_OBJECTS_H_
