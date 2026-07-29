#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "document.h"
#include "utils.h"

#define HEADER_PADDING 6
#define SCRAMBLE 0x24681357

static uint64_t conv64(uint64_t vlu)
{
    return ((0xFF00000000000000 & vlu) >> 56) |
           ((0x00FF000000000000 & vlu) >> 40) |
           ((0x0000FF0000000000 & vlu) >> 24) |
           ((0x000000FF00000000 & vlu) >> 8) |
           ((0x00000000FF000000 & vlu) << 8) |
           ((0x0000000000FF0000 & vlu) << 24) |
           ((0x000000000000FF00 & vlu) << 40) |
           ((0x00000000000000FF & vlu) << 56);
}

static uint64_t conv32(uint64_t vlu)
{
    return ((0xFF000000 & vlu) >> 24) |
           ((0x00FF0000 & vlu) >> 8) |
           ((0x0000FF00 & vlu) << 8) |
           ((0x000000FF & vlu) << 24);
}

static uint64_t conv16(uint64_t vlu)
{
    return ((0xFF00 & vlu) >> 8) |
           ((0x00FF & vlu) << 8);
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
        uint16_t: conv16(vlu),          \
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

static mem_Arena wr_arena;

static bool write_uint64(uint64_t vlu, FILE* wr_stream)
{
    vlu = TO_BYTE_ORDER(vlu);
    return fwrite(&vlu, 8, 1, wr_stream) == 1;
}

static bool write_uint32(uint32_t vlu, FILE* wr_stream)
{
    vlu = TO_BYTE_ORDER(vlu);
    return fwrite(&vlu, 4, 1, wr_stream) == 1;
}

static bool write_uint16(uint16_t vlu, FILE* wr_stream)
{
    vlu = TO_BYTE_ORDER(vlu);
    return fwrite(&vlu, 2, 1, wr_stream) == 1;
}

static bool write_uint8(uint8_t vlu, FILE* wr_stream)
{
    vlu = TO_BYTE_ORDER(vlu);
    return fwrite(&vlu, 1, 1, wr_stream) == 1;
}

static bool write_string(String* string, FILE* wr_stream)
{
    if (!write_uint32(string->size, wr_stream))
        return false;

    String scrambled;
    String_init(&scrambled, &wr_arena);
    if (!String_scramble(&scrambled, string, SCRAMBLE))
        return false;

    if (fwrite(scrambled.elements, 1, string->size, wr_stream) != string->size)
        return false;

    return true;
}

static bool write_person(Person* person, FILE* wr_stream)
{
    if (!write_uint32(person->roles_mask, wr_stream))
        return false;

    if (!write_string(&person->name, wr_stream))
        return false;

    if (!write_string(&person->email, wr_stream))
        return false;

    return true;
}

static bool write_test_atom(TestAtom* atom, FILE* wr_stream)
{
    if (!write_uint32(atom->type, wr_stream))
        return false;

    if (!write_uint32(atom->flags, wr_stream))
        return false;

    return write_string(&atom->string, wr_stream);
}

static bool write_test(Test* test, FILE* wr_stream)
{
    if (!write_string(&test->identifier, wr_stream))
        return false;

    if (!write_string(&test->command, wr_stream))
        return false;

    if (!write_uint32(test->atoms.size, wr_stream))
        return false;

    for (size_t i = 0; i < test->atoms.size; ++i) {
        if (!write_test_atom(&test->atoms.elements[i], wr_stream))
            return false;
    }

    return true;
}

static bool write_header(DocHeader *hdr, FILE* wr_stream)
{
    if (!write_uint32(hdr->identifier, wr_stream))
        return false;

    if (!write_uint32(hdr->byte_len, wr_stream))
        return false;

    time_t utc = time(NULL);
    if (!write_uint64((uint64_t)utc, wr_stream))
        return false;

    if (!write_uint8(hdr->compiler_person, wr_stream))
        return false;

    if (!write_uint8(hdr->version, wr_stream))
        return false;

    // padding
    uint8_t byte[HEADER_PADDING] = {0x5A};
    return fwrite(byte, 1, HEADER_PADDING, wr_stream) == HEADER_PADDING;
}

static bool write_projectname(String* project_name, FILE* write_stream)
{
    return write_string(project_name, write_stream);
}

static bool write_persons(PersonArr* persons, FILE* wr_stream)
{
    if (persons->size > 0xFF) {
        write_error("To many persons stored in document");
        return false;
    }

    if (!write_uint8(persons->size, wr_stream))
        return false;

    for (size_t i = 0; i < persons->size; ++i) {
        if (!write_person(&persons->elements[i], wr_stream))
            return false;
    }

    return true;
}

static bool write_sessions(TestArr* sessions, FILE* wr_stream)
{
    if (!write_uint32(sessions->size, wr_stream))
        return false;

    for (size_t i = 0; i < sessions->size; ++i) {
        if (!write_test(&sessions->elements[i], wr_stream))
            return false;
    }

    return true;
}

// ---------------------------------------------------------------
// read stuff

static bool read_uint64(uint64_t *vlu, FILE* rd_stream)
{
    if (fread(vlu, 8, 1, rd_stream) != 1)
        return false;
    *vlu = TO_BYTE_ORDER(*vlu);

    return true;
}

static bool read_uint32(uint32_t* vlu, FILE* rd_stream)
{
    if (fread(vlu, 4, 1, rd_stream) != 1)
        return false;
    *vlu = TO_BYTE_ORDER(*vlu);

    return true;
}

static bool read_uint16(uint16_t *vlu, FILE* rd_stream)
{
    if (fread(vlu, 2, 1, rd_stream) != 1)
        return false;
    *vlu = TO_BYTE_ORDER(*vlu);

    return true;
}

static bool read_uint8(uint8_t *vlu, FILE* rd_stream)
{
    if (fread(vlu, 1, 1, rd_stream) != 1)
        return false;
    *vlu = TO_BYTE_ORDER(*vlu);

    return true;
}

static bool read_string(String* string, FILE* rd_stream)
{
    uint32_t size;
    if (!read_uint32(&size, rd_stream))
        return false;
    String_resize(string, size+1);

    String scrambled;
    String_init(&scrambled, string->arena);
    if (!String_resize(&scrambled, size+1))
        return false;

    if (fread(scrambled.elements, 1, size, rd_stream) != size)
        return false;

    scrambled.size = size;

    if (!String_unscramble(string, &scrambled, SCRAMBLE))
        return false;


    return true;
}

static bool read_atom(TestAtom* atom, FILE* rd_stream)
{
    if (!read_uint32(&atom->type, rd_stream))
        return false;
    if (!read_uint32(&atom->flags, rd_stream))
        return false;

    return read_string(&atom->string, rd_stream);
}

static bool read_test(Test* test, FILE* rd_stream)
{
    if (!read_string(&test->identifier, rd_stream))
        return false;
    if (!read_string(&test->command, rd_stream))
        return false;

    uint32_t count;
    if (!read_uint32(&count, rd_stream))
        return false;

    for (uint32_t i = 0; i < count; ++i) {
        TestAtom atom;
        TestAtom_init(&atom, test->atoms.arena);
        if (!read_atom(&atom, rd_stream))
            return false;

        TestAtomArr_push_back(&test->atoms, atom);
    }

    return true;
}

static bool read_person(Person* person, FILE* rd_stream)
{
    if (!read_uint32(&person->roles_mask, rd_stream))
        return false;

    if (!read_string(&person->name, rd_stream))
        return false;

    return read_string(&person->email, rd_stream);
}

static bool read_header(Document* doc, FILE* rd_stream)
{
    DocHeader* hdr = &doc->header;

    if (!read_uint32(&hdr->identifier, rd_stream))
        return false;

    if (hdr->identifier != DOC_HDR_IDENTIFIER) {
        write_error("Document format not identified");
        return false;
    }

    // byte_size
    if (!read_uint32(&hdr->byte_len, rd_stream))
        return false;

    if (hdr->byte_len < sizeof(Document)) {
        write_error("Document size to small, must be malformed");
        return false;
    }

    // Timestamp
    if (!read_uint64(&hdr->date_compiled, rd_stream))
        return false;

    // compiler person idx
    if (!read_uint8(&hdr->compiler_person, rd_stream))
        return false;

    // version
    if (!read_uint8(&hdr->version, rd_stream))
        return false;

    // padding
    fseek(rd_stream, HEADER_PADDING, SEEK_CUR);

    return true;
}

static bool read_projectname(Document* doc, FILE* rd_stream)
{
    if (doc->header.version < 0x02)
        return false;
    return read_string(&doc->project_name, rd_stream);
}

static bool read_persons(Document* doc, FILE* rd_stream)
{
    PersonArr* persons = &doc->persons;

    uint8_t cnt;
    if (!read_uint8(&cnt, rd_stream))
        return false;
    if (!PersonArr_resize(persons, TO_BYTE_ORDER(cnt)))
        return false;

    for (uint8_t i = 0; i < cnt; ++i) {
        Person person;
        Person_init(&person, persons->arena);

        if (!read_person(&person, rd_stream))
            return false;

        if (!PersonArr_push_back(persons, person))
            return false;
    }

    return true;
}

static bool read_sessions(Document* doc, FILE* rd_stream)
{
    TestArr* sessions = &doc->test_sessions;

    uint32_t count;
    if (!read_uint32(&count, rd_stream))
        return false;

    for (uint32_t i = 0; i < count; ++i) {
        Test test;
        Test_init(&test, sessions->arena);

        if (!read_test(&test, rd_stream))
            return false;
        TestArr_push_back(sessions, test);
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
    String_init(&doc->project_name, arena);
    PersonArr_init(&doc->persons, arena);
    TestArr_init(&doc->test_sessions, arena);
}

Person Document_get_compiler_person(Document* doc)
{
    uint32_t idx = doc->header.compiler_person;
    if (idx >= doc->persons.size) {
        Person empty = {0};
        return empty;
    }

    return doc->persons.elements[idx];
}


bool Document_write(Document* doc, FILE* wr_stream, Person* compiler)
{
    (void)write_uint16; // currently unuseed

    int32_t idx = PersonArr_index_of(&doc->persons, *compiler);
    if (idx < 0 || idx > 0xFF) {
        write_error("Compiler person not found");
        return false;
    }

    mem_arena_init(&wr_arena);
    long start_pos = ftell(wr_stream);

    do { // bust out block
        if (!write_header(&doc->header, wr_stream))
            break;
        if (!write_projectname(&doc->project_name, wr_stream))
            break;
        if (!write_persons(&doc->persons, wr_stream))
            break;
        if (!write_sessions(&doc->test_sessions, wr_stream))
            break;

        // now that we know size, we save that in the header
        uint32_t bytes = ftell(wr_stream) - start_pos;
        fseek(wr_stream, start_pos+4, SEEK_SET);
        if (!write_uint32(bytes, wr_stream))
            break;
        fseek(wr_stream, 0, SEEK_END);

        mem_arena_free(&wr_arena);
        fflush(wr_stream);

        return true;

    } while(false);

    write_error("Failed to write document to stream");

    mem_arena_free(&wr_arena);

    return false;
}

bool Document_read(Document* doc, FILE* rd_stream)
{
    // need a fresh and initialized doc
    assert(doc->persons.arena != NULL);
    assert(doc->test_sessions.arena != NULL);
    assert(doc->header.byte_len == 0);

    (void)read_uint16; // currently unuseed

    do {
        if (!read_header(doc, rd_stream))
            break;
        if (!read_projectname(doc, rd_stream))
            break;
        if (!read_persons(doc, rd_stream))
            break;
        if (!read_sessions(doc, rd_stream))
            break;
        return true;

    } while (false);

    write_error("Failed to read in document");

    return false;
}