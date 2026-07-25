#include "arena.h"
#include "typestring.h"

#define NO_TEMPLATE_PERSON
#include "person.h"


#define T Person
#define NAME PersonArr
#define ARRAY_IMPLEMENTATION
#define CHECK_EQUAL(a,b) \
    a.roles_mask == b.roles_mask && \
    a.name.size == b.name.size && \
    strncmp(a.name.elements, b.name.elements, a.name.size) == 0 && \
    a.email.size == b.email.size && \
    strncmp(a.email.elements, b.email.elements, a.email.size) == 0
#include "array.template.h"



// ----------------------------------------------------------


void Person_init(Person *person, mem_Arena* arena)
{
    String_init(&person->name, arena);
    String_init(&person->email, arena);
    person->roles_mask = 0;
}