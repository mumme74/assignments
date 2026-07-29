#include <time.h>
#include "testrunner.h"
#include "document.h"
#include "arena.h"


#define check64(buf, right) \
    buf[0] == (right & 0xff00000000000000) >> 56 && \
    buf[1] == (right & 0x00ff000000000000) >> 48 && \
    buf[2] == (right & 0x0000ff0000000000) >> 40 && \
    buf[3] == (right & 0x000000ff00000000) >> 32 && \
    buf[4] == (right & 0x00000000ff000000) >> 24 && \
    buf[5] == (right & 0x0000000000ff0000) >> 16 && \
    buf[6] == (right & 0x000000000000ff00) >> 8 && \
    buf[7] == (right & 0x00000000000000ff) >> 0


#define check32(buf, right) \
    buf[0] == (right & 0xff000000) >> 24 && \
    buf[1] == (right & 0x00ff0000) >> 16 && \
    buf[2] == (right & 0x0000ff00) >> 8  && \
    buf[3] == (right & 0x000000ff) >> 0

#define checkString(buf, len, str, fp) \
    expectTrue(check32(buf, len)); \
    fread(buf, 1, buf[3], fp); \
    expectEQ((const char*)&buf[4], str)



// -----------------------------------------------------

TEST_SETUP(doc_suite)

static Document *doc = NULL;
static mem_Arena arena;

static const char filename[] = ".tmpFile";
static FILE* fileptr = NULL;

static void open_file(const char* modes)
{
    fileptr = fopen(filename, modes);
}

static void init_fill_doc(Document *doc)
{
    Document_init(doc, &arena);
    String_set(&doc->project_name, "TestProject", 11);

    Person p1, p2;
    Person_init(&p1, &arena);
    Person_init(&p2, &arena);
    String_append_str(&p1.name, "Person1", 7);
    String_append_str(&p2.name, "Person2", 7);

    PersonArr_push_back(&doc->persons, p1);
    PersonArr_push_back(&doc->persons, p2);


    Test test1, test2;
    Test_init(&test1, &arena);
    Test_init(&test2, &arena);
    String_append_str(&test1.identifier,"Test1", 5);
    String_append_str(&test2.identifier,"Test2", 5);
    String_append_str(&test1.command,"Cmd1", 4);
    String_append_str(&test2.command,"Cmd2", 4);


    TestAtom a1, a2, a3, a4;
    TestAtom_init(&a1, &arena);
    TestAtom_init(&a2, &arena);
    TestAtom_init(&a3, &arena);
    TestAtom_init(&a4, &arena);
    String_append_str(&a1.string, "Atom1", 5);
    String_append_str(&a2.string, "Atom2", 5);
    String_append_str(&a3.string, "Atom3", 5);
    String_append_str(&a4.string, "Atom4", 5);
    a1.flags = 0xa5;
    a2.flags = 0xa6;
    a3.flags = 0xa7;
    a4.flags = 0xa8;
    a1.type = Argv_Type;
    a2.type = Stdin_Type;
    a3.type = Stdout_Type;
    a4.type = Stdin_Type | Stdout_Type;
    TestAtomArr_push_back(&test1.atoms, a1);
    TestAtomArr_push_back(&test1.atoms, a2);
    TestAtomArr_push_back(&test2.atoms, a3);
    TestAtomArr_push_back(&test2.atoms, a4);


    TestArr_push_back(&doc->test_sessions, test1);
    TestArr_push_back(&doc->test_sessions, test2);

    doc->header.compiler_person = 1;
}

static void createDocFile()
{
    Document doc;
    init_fill_doc(&doc);

    open_file("w");
    Document_write(&doc, fileptr, &doc.persons.elements[1]);
    fclose(fileptr);

    memset(&doc, 0, sizeof(Document));
    Document_init(&doc, &arena);

    fileptr = NULL;
}

TEST_SUITE_SETUP_FN(doc_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(doc_suite)
{
    mem_arena_free(&arena);
}

TEST_SETUP_FN(doc_suite)
{
    doc = mem_arena_alloc(&arena, sizeof(Document));
    Document_init(doc, &arena);
}

TEST_TEARDOWN_FN(doc_suite)
{
    if (fileptr) {
        fclose(fileptr);
        unlink(filename);
        fileptr = NULL;
    }
}

TEST(doc_suite, doc_init, "Should init document")
{
    expectEQ(doc->header.identifier, DOC_HDR_IDENTIFIER);
    expectEQ(doc->header.version, DOC_HDR_VERSION);
    expectEQ(doc->header.byte_len, 0);
    expectEQ(doc->header.compiler_person, 0);
    expectEQ(doc->header.date_compiled, 0);

    expectEQ(doc->persons.size, 0);
    expectEQ((void*)doc->persons.arena, &arena);
    expectEQ(doc->test_sessions.size, 0);
    expectEQ((void*)doc->test_sessions.arena, &arena);
}

TEST(doc_suite, doc_save, "Should save doc")
{
    init_fill_doc(doc);

    open_file("w");
    expectTrue(Document_write(doc, fileptr, &doc->persons.elements[1]));

    long supposedEnd = ftell(fileptr);
    fseek(fileptr, 0, SEEK_END);
    long actualend = ftell(fileptr);
    expectEQ(supposedEnd, actualend);

    fclose(fileptr);
    open_file("r");
    long start = ftell(fileptr);

    // identifier / Byteorder mark
    uint8_t buf[100] = {0};
    fread(buf, 1, 4, fileptr);
    expectEQ(buf[0], 0xFA);
    expectEQ(buf[1], 0xCE);
    expectEQ(buf[2], 0xFE);
    expectEQ(buf[3], 0xED);

    // size
    fseek(fileptr, 0, SEEK_END);
    long end = ftell(fileptr);
    fseek(fileptr, 4, SEEK_SET);
    fread(buf, 1, 4, fileptr);
    long size = end - start;
    expectTrue(check32(buf, size));

    // timestamp
    fread(buf, 8, 1, fileptr);
    uint64_t zero = 0;
    expectFalse(check64(buf, zero));

    // compiler person
    fread(buf, 1, 1, fileptr);
    expectEQ(buf[0], 1);

    // version
    fread(buf, 1, 1, fileptr);
    expectEQ(buf[0], DOC_HDR_VERSION);

    // Project name size
    fseek(fileptr, sizeof(DocHeader), SEEK_SET);
    fread(buf, 1, 4, fileptr);
    expectTrue(check32(buf, 11));

    // project name
    fread((char*)&buf[4], 1, 11, fileptr);
    expectNE((char*)&buf[4], "TestProject");
    String *name = String_new(&arena),
           *scrambled = String_new(&arena);
    String_set(scrambled, (char*)&buf[4], 11);
    String_unscramble(name, scrambled, DOC_SCRAMBLE);

    // person cnt
    fseek(fileptr, sizeof(DocHeader) + 4 + buf[3], SEEK_SET);
    fread(buf, 1, 1, fileptr);
    expectEQ(buf[0], 2);

}


TEST(doc_suite, doc_read, "Should read doc")
{
    createDocFile();

    open_file("r");
    expectTrue(Document_read(doc, fileptr));

    expectEQ(doc->header.identifier, DOC_HDR_IDENTIFIER);
    expectGT(doc->header.byte_len, sizeof(Document));
    time_t now = time(NULL);
    expectLT(doc->header.date_compiled, now+1);
    expectGT(doc->header.date_compiled, now-60);
    expectEQ(doc->header.compiler_person, 1);
    expectEQ(doc->header.version, DOC_HDR_VERSION);

    expectEQ(doc->project_name.elements, "TestProject");

    expectEQ(doc->persons.size, 2);
    expectEQ(doc->persons.elements[0].name.elements, "Person1");
    expectEQ(doc->persons.elements[1].name.elements, "Person2");

    expectEQ(doc->test_sessions.size, 2);
    Test *test1 = &doc->test_sessions.elements[0];
    expectEQ(test1->identifier.elements, "Test1");
    expectEQ(test1->command.elements, "Cmd1");
    expectEQ(test1->atoms.size, 2);
    Test *test2 = &doc->test_sessions.elements[1];
    expectEQ(test2->identifier.elements, "Test2");
    expectEQ(test2->command.elements, "Cmd2");
    expectEQ(test2->atoms.size, 2);

    TestAtom *atom = &test1->atoms.elements[0];
    expectEQ(atom->string.elements, "Atom1");
    expectEQ(atom->flags, 0xa5);
    expectEQ(atom->type, Argv_Type);
    atom = &test1->atoms.elements[1];
    expectEQ(atom->string.elements, "Atom2");
    expectEQ(atom->flags, 0xa6);
    expectEQ(atom->type, Stdin_Type);


    atom = &test2->atoms.elements[0];
    expectEQ(atom->string.elements, "Atom3");
    expectEQ(atom->flags, 0xa7);
    expectEQ(atom->type, Stdout_Type);
    atom = &test2->atoms.elements[1];
    expectEQ(atom->string.elements, "Atom4");
    expectEQ(atom->flags, 0xa8);
    expectEQ(atom->type, Stdin_Type | Stdout_Type);

}
