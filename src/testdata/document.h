#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include <stdint.h>
#include "typestring.h"
#include "person.h"
#include "tests.h"

#define DOC_HDR_IDENTIFIER 0xFACEFEED
#define DOC_HDR_VERSION 0x01

/*

Structure of doc
+------------------------------+
| doc_Header                   |
+------------------------------+
| doc_Persons                  |
+------------------------------+
| doc_Person 1                 |
| doc_Person 2                 |
| ...                          |
+------------------------------+
| doc_Tests                    |
+------------------------------+
| doc_Test 1                   |
|     doc_Obj1 (input)         |
|     doc_Obj2 (expected)      |
| doc_Test 2                   |
|     doc_Obj1 (input)         |
|     doc_Obj2 (expected)      |
| ...                          |
+------------------------------+

*/


/**
 * The header of for the binary format
 */
typedef struct DocHeader {
    uint32_t identifier; ///< Identifier for big/little endian
    uint32_t byte_len; ///< The length in bytes of this blob, excluding header
    uint64_t date_compiled; ///< A UTC timestamp when this was compiled
    uint8_t  compiler_person; ///< The person that compiled this blob
    uint8_t  version; ///< The version of the storageformat of this data blob
    uint8_t _pad[5]; // align to 8bytes multiples
} DocHeader;


/**
 * A complete document
 */
typedef struct Document {
    DocHeader header;
    PersonArr persons;
    TestArr test_sessions;
} Document;


// --------------------------------------------------------------
void Document_header_init(DocHeader *header);

// --------------------------------------------------------------
void Document_init(Document *doc, mem_Arena* arena);



#endif // _DOCUMENT_H_
