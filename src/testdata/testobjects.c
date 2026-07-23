#include <stdlib.h>
#include <string.h>
#include "testobjects.h"


// ----------------------------------------------------------


void testdata_person_init(testdata_Person *person)
{
    memset(person, 0, sizeof(*person));
}

// -------------------------------------------------

void testdata_persons_init(testdata_Persons *persons)
{
    memset(persons, 0, sizeof(*persons));
}

bool testdata_persons_pre_alloc(
    testdata_Persons* persons, uint16_t cnt, mem_Arena* arena
) {
    testdata_persons_init(persons);

    size_t sz = sizeof(testdata_Person*) * cnt;
    persons->data = mem_arena_alloc(arena, (uint32_t)sz);
    if (!persons->data)
        return false;

    memset(persons->data, 0, sz);

    persons->size = cnt;
    persons->len = 0;

    return true;
}

int32_t testdata_persons_add_person(
    testdata_Persons* persons, testdata_Person* person, mem_Arena* arena
) {
    if (persons->size <= persons->len) {
        uint32_t oldLen = persons->len;
        testdata_Person** oldPersons = persons->data;

        if (!testdata_persons_pre_alloc(persons, oldLen+1, arena))
            return -1;

        for (size_t i = 0; i < oldLen; ++i) {
            persons->data[i] = oldPersons[i];
        }
        persons->len = oldLen;
    }

    persons->data[persons->len++] = person;

    return persons->len - 1;
}

int32_t testdata_persons_insert(
    testdata_Persons* persons, testdata_Person* person,
    testdata_Person* before, mem_Arena* arena
) {
    if (persons->len == 0 || before == NULL)
        return testdata_persons_add_person(persons, person, arena);


    int32_t idx = testdata_persons_index_of(persons, before);
    if (idx < 0) return -1;

    // first add last to make room for one more.
    int32_t idx2 = testdata_persons_add_person(persons, person, arena);
    if (idx2 < 0) return -1;

    for (int32_t i = persons->len-1, j = persons->len-2;
         i > idx; --i, --j
    ) {
        persons->data[i] = persons->data[j];
    }
    persons->data[idx] = person;

    return idx;
}

int32_t testdata_persons_index_of(
    testdata_Persons* persons, testdata_Person* person
) {
    for (uint32_t i = 0; i < persons->len; ++i) {
        if (persons->data[i] == person)
            return i;
    }

    return -1;
}

bool testdata_persons_remove(
    testdata_Persons* persons, testdata_Person* person
) {
    int32_t idx = testdata_persons_index_of(persons, person);
    if (idx < 0)
        return false;

    for (size_t i = idx, j= idx+1; j < persons->len; ++i, ++j) {
        persons->data[i] = persons->data[j];
    }

    persons->len -= 1;

    return true;
}

// ------------------------------------------------


void testdata_obj_init(testdata_Obj *obj)
{
    memset(obj, 0, sizeof(*obj));
}

// ------------------------------------------------

void testdata_test_init(testdata_Test *test)
{
    memset(test, 0, sizeof(*test));
}

bool testdata_test_pre_alloc(
    testdata_Test* test, uint16_t cnt, mem_Arena* arena
) {
    testdata_test_init(test);

    size_t sz = sizeof(testdata_Person*) * cnt;
    test->data = mem_arena_alloc(arena, (uint32_t)sz);
    if (!test->data)
        return false;

    memset(test->data, 0, sz);

    test->size = cnt;
    test->len = 0;

    return true;
}

int32_t testdata_test_add_obj(
    testdata_Test* test, testdata_Obj* obj, mem_Arena* arena
) {
    if (test->size <= test->len) {
        uint32_t oldLen = test->len;
        testdata_Obj** oldObj = test->data;

        if (!testdata_test_pre_alloc(test, oldLen+1, arena))
            return -1;

        for (size_t i = 0; i < oldLen; ++i) {
            test->data[i] = oldObj[i];
        }
        test->len = oldLen;
    }

    test->data[test->len++] = obj;

    return test->len - 1;
}

int32_t testdata_test_insert(
    testdata_Test* test, testdata_Obj* obj,
    testdata_Obj* before, mem_Arena* arena
) {
    if (test->len == 0 || before == NULL)
        return testdata_test_add_obj(test, obj, arena);


    int32_t idx = testdata_test_index_of(test, before);
    if (idx < 0) return -1;

    // first add last to make room for one more.
    int32_t idx2 = testdata_test_add_obj(test, obj, arena);
    if (idx2 < 0) return -1;

    for (int32_t i = test->len-1, j = test->len-2;
         i > idx; --i, --j
    ) {
        test->data[i] = test->data[j];
    }
    test->data[idx] = obj;

    return idx;
}

int32_t testdata_test_index_of(
    testdata_Test* test, testdata_Obj* obj
) {
    for (uint32_t i = 0; i < test->len; ++i) {
        if (test->data[i] == obj)
            return i;
    }

    return -1;
}

bool testdata_test_remove(
    testdata_Test* test, testdata_Obj* obj
) {
    int32_t idx = testdata_test_index_of(test, obj);
    if (idx < 0)
        return false;

    for (size_t i = idx, j= idx+1; j < test->len; ++i, ++j) {
        test->data[i] = test->data[j];
    }

    test->len -= 1;

    return true;
}

// ------------------------------------------------

void testdata_tests_init(testdata_Tests *tests)
{
    memset(tests, 0, sizeof(*tests));
}


bool testdata_tests_pre_alloc(
    testdata_Tests* tests, uint16_t cnt, mem_Arena* arena
) {
    testdata_tests_init(tests);

    size_t sz = sizeof(testdata_Person*) * cnt;
    tests->data = mem_arena_alloc(arena, (uint32_t)sz);
    if (!tests->data)
        return false;

    memset(tests->data, 0, sz);

    tests->size = cnt;
    tests->len = 0;

    return true;
}

int32_t testdata_tests_add_obj(
    testdata_Tests* tests, testdata_Test* test, mem_Arena* arena
) {
    if (tests->size <= tests->len) {
        uint32_t oldLen = tests->len;
        testdata_Test** oldObj = tests->data;

        if (!testdata_tests_pre_alloc(tests, oldLen+1, arena))
            return -1;

        for (size_t i = 0; i < oldLen; ++i) {
            tests->data[i] = oldObj[i];
        }
        tests->len = oldLen;
    }

    tests->data[tests->len++] = test;

    return tests->len - 1;
}

int32_t testdata_tests_insert(
    testdata_Tests* tests, testdata_Test* test,
    testdata_Test* before, mem_Arena* arena
) {
    if (tests->len == 0 || before == NULL)
        return testdata_tests_add_obj(tests, test, arena);


    int32_t idx = testdata_tests_index_of(tests, before);
    if (idx < 0) return -1;

    // first add last to make room for one more.
    int32_t idx2 = testdata_tests_add_obj(tests, test, arena);
    if (idx2 < 0) return -1;

    for (int32_t i = tests->len-1, j = tests->len-2;
         i > idx; --i, --j
    ) {
        tests->data[i] = tests->data[j];
    }
    tests->data[idx] = test;

    return idx;
}

int32_t testdata_tests_index_of(
    testdata_Tests* tests, testdata_Test* test
) {
    for (uint32_t i = 0; i < tests->len; ++i) {
        if (tests->data[i] == test)
            return i;
    }

    return -1;
}

bool testdata_tests_remove(
    testdata_Tests* tests, testdata_Test* test
) {
    int32_t idx = testdata_tests_index_of(tests, test);
    if (idx < 0)
        return false;

    for (size_t i = idx, j= idx+1; j < test->len; ++i, ++j) {
        test->data[i] = test->data[j];
    }

    test->len -= 1;

    return true;
}

// -------------------------------------------------


void testdata_doc_header_init(testdata_Header *hdr)
{
    memset(hdr, 0, sizeof(*hdr));
}

//-----------------------------------------------------------------

void testdata_doc_init(testdata_Document *doc)
{
    testdata_doc_header_init(&doc->header);
    testdata_persons_init(&doc->persons_hdr);
    doc->persons = NULL;
    testdata_tests_init(&doc->tests_list_hdr);
    doc->tests = NULL;
}

void testdata_Person_add_person(
    testdata_Document *doc, testdata_Person *person)
{
    (void)doc;
    (void)person;

}

// ----------------------------------------------------------------

