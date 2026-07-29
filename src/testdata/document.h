#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include <stdio.h>
#include <stdint.h>
#include "typestring.h"
#include "person.h"
#include "tests.h"

#define DOC_HDR_IDENTIFIER 0xFACEFEED
#define DOC_HDR_IDENTIFIER_REVERSE 0xEDFEEFFA
#define DOC_HDR_VERSION 0x02

/*

Structure of doc
+--------------------------+
| Header                   |
+--------------------------+
| Project name             |
+--------------------------+
| Persons                  |
+--------------------------+
| Person 1                 |
| Person 2                 |
| ...                      |
+--------------------------+
| Tests                    |
+--------------------------+
| Test 1                   |
|     Atom1 (input)        |
|     Atom22 (expected)    |
| Test 2                   |
|     Atom1 (input)        |
|     Atom2 (expected)     |
| ...                      |
+--------------------------+


Document binary format:
Every value is endian swapped to bigendian


Header:
   uint32_t identifier not ENDIAN swapped
   uint32_t bytelen of this document blob
   uint64_t timestamp compiled in unix epoch
   uint8_t  index in persons array that compiled the blob
   uint8_t  version format of this blob
   uint8_t[6] paddingto 8bytes multiples, might be taken in future

PersonsArr
    uint8_t number of persons in this array

    Person (repeating)
        uint32_t Roles mask
        String name
        String email

TestArr
    uint32_t number of test sessions in this array

    Test (repeating)
        String identifier
        String command
        uint32_t number of atoms in this test

        TestAtom (repeating)
            uint32_t Test type
            uint32_t flags
            String string The thing to do or compare.




String (spread throughout)
    uint32_t strlen
    char[*] null terminaed string (scrambled)
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
    String project_name;
    PersonArr persons;
    TestArr test_sessions;
} Document;


// --------------------------------------------------------------
void Document_header_init(DocHeader *header);

// --------------------------------------------------------------
void Document_init(Document *doc, mem_Arena* arena);

/**
 * Get the person that compiled this document
 */
Person Document_get_compiler_person(Document* doc);

/**
 * Write this document to filestream
 *
 * @param doc this document to write
 * @param wr_stream The file stream to write to
 * @param compiler The person that writes this doc to wr_stream
 */
bool Document_write(Document *doc, FILE* wr_stream, Person* compiler);


/**
 * Read a new document from rd_stream
 */
bool Document_read(Document *doc, FILE* rd_stream);



#endif // _DOCUMENT_H_
