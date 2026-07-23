#include <stdlib.h>
#include <string.h>
#include "testobjects.h"


// ----------------------------------------------------------


void testdata_Header_init(testdata_Header *hdr)
{
    memset(hdr, 0, sizeof(*hdr));
}

void testdata_Persons_init(testdata_Persons *persons)
{
    memset(persons, 0, sizeof(*persons));
}

void testdata_Person_init(testdata_Person *person)
{
    memset(person, 0, sizeof(*person));
}

void testdata_Tests_init(testdata_Tests *tests)
{
    memset(tests, 0, sizeof(*tests));
}

void testdata_Obj_init(testdata_Obj *obj)
{
    memset(obj, 0, sizeof(*obj));
}

void testdata_Document_init(testdata_Document *doc)
{
    testdata_Header_init(&doc->header);
    testdata_Persons_init(&doc->persons_hdr);
    doc->persons = NULL;
    testdata_Tests_init(&doc->tests_list_hdr);
    doc->tests = NULL;
}


void testdata_Person_add_person(
    testdata_Document *doc, testdata_Person *person)
{
    (void)doc;
    (void)person;

}

