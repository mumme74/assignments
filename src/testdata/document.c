#include <stdlib.h>
#include <string.h>

#include "document.h"


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
