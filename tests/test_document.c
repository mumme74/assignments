#include "testrunner.h"
#include "document.h"
#include "arena.h"


// -----------------------------------------------------

TEST_SETUP(doc_suite)

static Document *doc = NULL;
static mem_Arena arena;

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
