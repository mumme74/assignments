#ifndef _TEST_OBJECTS_H_
#define _TEST_OBJECTS_H_

#include <stdint.h>

#define TESTDATA_HDR_IDENTIFIER 0xFACEFEED
#define TESTDATA_HDR_VERSION 0x01

/*

Structure of testData
+------------------------------+
| testdata_header              |
+------------------------------+
| testdata_persons_list_hdr    |
+------------------------------+
| testdata_person 1            |
| testdata_person 2            |
| ...                          |
+------------------------------+
| testdata_testlist_hdr        |
+------------------------------+
| testdata_test 1              |
|     testdata_obj1 (input)    |
|     testdata_obj2 (expected) |
| testdata_test 2              |
|     testdata_obj1 (input)    |
|     testdata_obj2 (expected) |
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
 * Stores a string in a scrambled form
 */
struct testdata_string {
    uint32_t len; ///< how many bytes
    uint8_t *data; ///< Holds len amounts of str data, null terminated
};

/**
 * The header of for the binary format
 */
struct testdata_header {
    uint32_t identifier; ///< Identifier for big/little endian
    uint32_t byte_len; ///< The length in bytes of this blob, excluding header
    uint64_t date_compiled; ///< A UTC timestamp when this was compiled
    uint8_t  compiler_person; ///< The person that compiled this blob
    uint8_t  version; ///< The version of the storageformat of this data blob
    uint8_t _pad[5]; // align to 8bytes multiples
};

/**
 * The header for all persons list
 */
struct testdata_persons_list_hdr {
    uint32_t _next_section; ///< For internal use, Num bytes in this section +1
    uint16_t num_persons; ///< How many persons in this list
};

/**
 * A single person
 */
struct testdata_person
{
    uint8_t roles_mask; ///< The testdata_person_Roles this person has
    struct testdata_string name; ///< the name of this person.
    struct testdata_string email; ///< the email of this person.
};


/**
 * The actual tests header
 */
struct testdata_tests_list_hdr
{
    uint32_t _next_section; ///< For internal use, Num bytes in this section +1
    uint16_t num_tests; ///< How many tests stored
};

/**
 * A specific test session
 */
struct testdata_test {
    struct testdata_string identifier; ///< A identifier for this test
    struct testdata_string command; ///< The command to run on the client
    uint16_t num_test_objs; ///< The number of testObjects in this test
};

/**
 * A atomic thing to do or expect during a test session
 */
struct testdata_test_obj {
    enum testdata_Type type; ///< The type of test to perform
    struct testdata_string data; ///< the string to send or expect
    uint16_t flags;  ///< Controlling flags for this test
};


/**
 * A complete document in the order as its structured.
 */
struct testdata_document {
    struct testdata_header header;
    struct testdata_persons_list_hdr persons_hdr;
    struct testdata_person *persons;
    struct testdata_tests_list_hdr tests_list_hdr;
    struct testdata_test *tests;
};

// -----------------------------------------------------------------------

void testdata_init_header(struct testdata_header *header);
void testdata_init_persons_list_hdr(struct testdata_personslist_*hdr);
void testdata_init_person(struct testdata_person *person);
void testdata_init_tests_list_hdr(struct testdata_tests_list_hdr *hdr);
void testdata_init_test_obj(struct testdata_test_obj *obj);
void testdata_init_document(struct testdata_document *doc);





#endif // _TEST_OBJECTS_H_
