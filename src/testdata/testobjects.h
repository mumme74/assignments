#ifndef _TEST_OBJECTS_H_
#define _TEST_OBJECTS_H_

#include <stdint.h>
#include "typestring.h"

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
 * A single person
 */
typedef struct
{
    uint8_t roles_mask; ///< The testdata_person_Roles this person has
    types_String name; ///< the name of this person.
    types_String email; ///< the email of this person.
} testdata_Person;

/**
 * The header for all persons list
 */
typedef struct {
    uint32_t size; ///< Allocated number of persons
    uint32_t len; ///< Num of persons in this doc
    testdata_Person **data; ///< Array of persons
} testdata_Persons;

/**
 * A atomic thing to do or expect during a test session
 */
typedef struct {
    enum testdata_Type type; ///< The type of test to perform
    types_String string; ///< the string to send or expect
    uint16_t flags;  ///< Controlling flags for this test
} testdata_Obj;

/**
 * A specific test session
 */
typedef struct {
    types_String identifier; ///< A identifier for this test
    types_String command; ///< The command to run on the client
    uint16_t size; ///< The allocated cnt
    uint16_t len; ///< The number of testObjects in this test
    testdata_Obj** data; ///< The test objects for this test
} testdata_Test;

/**
 * The actual tests header
 */
typedef struct
{
    uint16_t size; ///< How many test is allocated in this tests list
    uint16_t len; ///< How many test is stored.
    testdata_Test** data;
} testdata_Tests;

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

void testdata_person_init(testdata_Person *person);

// --------------------------------------------------------------
/**
 * Initialize a list of persons
 *
 * @param persons The persons list
 */
void testdata_persons_init(testdata_Persons *persons);

/**
 * Pre-allocate cnt number of persons with arena
 *
 * @param persons The list of persons
 * @param cnt How many to persons this list should hold.
 * @param arena The memory arena to allocate on
 * @returns false if failed.
 */
bool testdata_persons_pre_alloc(
    testdata_Persons* persons, uint16_t cnt, mem_Arena* arena);

/**
 * Add person to the list of persons, grow list if needed.
 *
 * @param persons The list to add the person to.
 * @param person The person to add
 * @param arena The arena to allocate on.
 * @return index in list for the newly added or -1 if failed
 */
int32_t testdata_persons_add_person(
    testdata_Persons* persons, testdata_Person* person, mem_Arena* arena);


/**
 * Insert person into persons list before other person
 *
 * @param persons The list to insert person into
 * @param person The person to insert
 * @param before Before this person, may be NULL which inserts last.
 * @param arena Allocate using this arena
 * @return index of inserted person or -1 if failed
 */
int32_t testdata_persons_insert(
    testdata_Persons* persons, testdata_Person* person,
    testdata_Person* before, mem_Arena* arena);

/**
 * Get the index of this person in the list of persons
 *
 * @param persons The list of persons to search in
 * @param person The person to look for
 * @return the index o -1 if not found.
 */
int32_t testdata_persons_index_of(
    testdata_Persons* persons, testdata_Person* person);

/**
 * Remove the person from persons list.
 *
 * @param persons The list to remove person from.
 * @param person The person to remove.
 * @return false if failed, like not found
 */
bool testdata_persons_remove(
    testdata_Persons* persons, testdata_Person* person);

// --------------------------------------------------------------
void testdata_obj_init(testdata_Obj *obj);


// ---------------------------------------------------------------


void testdata_test_init(testdata_Test *test);


bool testdata_test_pre_alloc(
    testdata_Test* test, uint16_t cnt, mem_Arena* arena);

int32_t testdata_test_add_obj(
    testdata_Test* test, testdata_Obj* obj, mem_Arena* arena);


int32_t testdata_test_insert(
    testdata_Test* test, testdata_Obj* obj,
    testdata_Obj* before, mem_Arena* arena);

int32_t testdata_test_index_of(
    testdata_Test* test, testdata_Obj* obj);

bool testdata_test_remove(
    testdata_Test* test, testdata_Obj* obj);

//  ---------------------------------------------------------------
void testdata_tests_init(testdata_Tests *tests);

bool testdata_tests_pre_alloc(
    testdata_Tests* tests, uint16_t cnt, mem_Arena* arena);

int32_t testdata_tests_add_obj(
    testdata_Tests* tests, testdata_Test* test, mem_Arena* arena);

int32_t testdata_tests_insert(
    testdata_Tests* tests, testdata_Test* test,
    testdata_Test* before, mem_Arena* arena);

int32_t testdata_tests_index_of(
    testdata_Tests* tests, testdata_Test* test);

bool testdata_tests_remove(
    testdata_Tests* tests, testdata_Test* test);

// --------------------------------------------------------------
void testdata_doc_header_init(testdata_Header *header);

// --------------------------------------------------------------
void testdata_doc_init(testdata_Document *doc);

bool testdata_doc_add_persons();

/**
 * Add a person to the document
 *
 * @param doc The document to add the person to
 */
void testdata_Document_add_person(
    testdata_Document *doc, testdata_Person *person);





#endif // _TEST_OBJECTS_H_
