#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "document.h"
#include "utils.h"

static uint64_t conv64(uint64_t vlu)
{
    return ((0xFF00000000000000 & vlu) >> 56) |
           ((0x00FF000000000000 & vlu) >> 48) |
           ((0x0000FF0000000000 & vlu) << 40) |
           ((0x000000FF00000000 & vlu) << 32) |
           ((0x00000000FF000000 & vlu) << 24) |
           ((0x0000000000FF0000 & vlu) << 16) |
           ((0x000000000000FF00 & vlu) << 8) |
           ((0x00000000000000FF & vlu) << 0);
}

static uint64_t conv32(uint64_t vlu)
{
    return ((0xFF000000 & vlu) << 24) |
           ((0x00FF0000 & vlu) << 16) |
           ((0x0000FF00 & vlu) << 8) |
           ((0x000000FF & vlu) << 0);
}

static uint64_t conv16(uint64_t vlu)
{
    return ((0xFF00 & vlu) << 16) |
           ((0x00FF & vlu) << 0);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    /* System is Little Endian */
# define TO_BYTE_ORDER(vlu)             \
    _Generic((vlu),                     \
        uint64_t: conv64(vlu),          \
        int64_t: conv64((uint64_t)vlu), \
        double:  conv64((uint64_t)vlu), \
        uint32_t: conv32(vlu),          \
        int32_t: conv32((uint32_t)vlu), \
        float: conv32((uint32_t)vlu),   \
        uint16_t: conv(vlu),            \
        int16_t: conv16(vlu),           \
        uint8_t: vlu,                   \
        int8_t:  vlu,                   \
        char:    vlu,                   \
        const char: vlu                 \
    )
#else
    /* System is Big Endian */
# define TO_BYTE_ORDER(vlu) vlu

#endif

static bool write_string(String* string, FILE* wr_stream)
{
    uint32_t size = TO_BYTE_ORDER(string->size),
             len = string->size+1;

    if (fwrite(size, sizeof(size), 1, wr_stream) != sizeof(size))
        return false;

    if (fwrite(string->elements, 1, len, wr_stream) != len)
        return false;

}

static bool write_person(Person* person, FILE* wr_stream)
{
    uint32_t roles = TO_BYTE_ORDER(person->roles_mask);
    if (fwrite(roles, sizeof(roles), 1, wr_stream) != sizeof(roles))
        return false;

    if (!write_string(&person->name, wr_stream))
        return false;

    if (!write_string(&person->email, wr_stream))
        return false;

    return true;
}

static bool write_test_atom(TestAtom* atom, FILE* wr_stream)
{
    uint32_t type = TO_BYTE_ORDER(atom->type),
             flags = TO_BYTE_ORDER(atom->flags);

    if (fwrite(type, sizeof(type), 1, wr_stream) != sizeof(type))
        return false;

    if (fwrite(flags, sizeof(flags), 1, wr_stream) != sizeof(flags))
        return false;

    return write_string(&atom->string, wr_stream);
}

static bool write_test(Test* test, FILE* wr_stream)
{
    if (!write_string(&test->identifier, wr_stream))
        return false;

    if (!write_string(&test->command, wr_stream))
        return false;

    for (size_t i = 0; i < test->atoms.size; ++i) {
        if (!write_test_atom(&test->atoms.elements[i], wr_stream))
            return false;
    }

    return true;
}

static bool write_header(DocHeader *hdr, FILE* wr_stream)
{
    if (fwrite(hdr->identifier, sizeof(hdr->identifier), 1, wr_stream) != 4)
        return false;

    if (fwrite(TO_BYTE_ORDER(hdr->byte_len),
               sizeof(hdr->byte_len), 1, wr_stream) != sizeof(hdr->byte_len)
    )
        return false;

    time_t utc = time(NULL);
    if (fwrite(TO_BYTE_ORDER(utc), sizeof(utc), 1, wr_stream) != sizeof(utc))
        return false;

    if (fwrite((uint8_t)hdr->compiler_person, 1, 1, wr_stream) != 1)
        return false;

    if (fwrite((uint8_t)hdr->version, 1, 1, wr_stream) != 1)
        return false;

    // padding
    return fwrite((uint8_t)0x5A, 1, 5, wr_stream) == 5;
}

static bool write_persons(PersonArr* persons, FILE* wr_stream)
{
    if (persons->size > 0xFF) {
        write_error("To many persons stored in document");
        return false;
    }

    uint8_t person_cnt = (uint8_t)persons->size;
    if (fwrite(TO_BYTE_ORDER(person_cnt), 1, 1, wr_stream) != 1)
        return false;

    for (size_t i = 0; i < persons->size; ++i) {
        if (!write_person(&persons->elements[i], wr_stream))
            return false;
    }

    return true;
}

static bool write_sessions(TestArr* sessions, FILE* wr_stream)
{
    uint32_t count = TO_BYTE_ORDER(sessions->size);
    if (fwrite(count, sizeof(count), 1, wr_stream) != sizeof(count))
        return false;

    for (size_t i = 0; i < sessions->size; ++i) {
        if (!write_test(&sessions->elements[i], wr_stream))
            return false;
    }

    return true;
}

//----------------------------------------------------------------


void Document_header_init(DocHeader *hdr)
{
    hdr->identifier = DOC_HDR_IDENTIFIER;
    hdr->version = DOC_HDR_VERSION;
    hdr->compiler_person = 0;
    hdr->date_compiled = 0;
    hdr->byte_len = 0;
}

//-----------------------------------------------------------------

void Document_init(Document *doc, mem_Arena* arena)
{
    Document_header_init(&doc->header);
    PersonArr_init(&doc->persons, arena);
    TestArr_init(&doc->test_sessions, arena);
}

Person Document_get_compiler_person(Document* doc)
{
    uint32_t idx = doc->header.compiler_person;
    if (idx < 0 || idx >= doc->persons.size) {
        Person empty = {0};
        return empty;
    }

    return doc->persons.elements[idx];
}


bool Document_write(Document* doc, FILE* wr_stream, Person* compiler)
{
    int32_t idx = PersonArr_index_of(&doc->persons, *compiler);
    if (idx < 0 || idx > 0xFF) {
        write_error("Compiler person not found");
        return false;
    }

    if (!write_header(&doc->header, wr_stream) ||
        !write_persons(&doc->persons, wr_stream) ||
        !write_sessions(&doc->test_sessions, wr_stream))
    {
        write_error("Failed to write document to stream");
        return false;
    }

    return true;
}