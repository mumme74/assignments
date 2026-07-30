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


static const char *role_names[4] = {
    "TeacherRole", "StudentRole", "ExternalRole", "OtherRole"
};


// ----------------------------------------------------------



const char* person_role_to_str(enum PersonRoles role)
{
    switch (role) {
    case TeacherRole:  return role_names[0];
    case StudentRole:  return role_names[1];
    case ExternalRole: return role_names[2];
    case OtherRole:    return role_names[3];
    default: return NULL;
    }
}


enum PersonRoles person_str_to_role(const char* role_str)
{
    if (!role_str) return 0;

    for (size_t i = 0; i < sizeof(role_names)/sizeof(role_names[0]); ++i) {
        if (strcmp(role_names[i], role_str) == 0)
            return 0x01 << i;
    }
    return 0;
}


void Person_init(Person *person, mem_Arena* arena)
{
    String_init(&person->name, arena);
    String_init(&person->email, arena);
    person->roles_mask = 0;
}

StringArr* Person_roles(Person *person, mem_Arena* arena)
{
    StringArr* arr = (StringArr*)mem_arena_alloc(arena, sizeof(StringArr));
    if (!arr) return NULL;

    StringArr_init(arr, arena);

    for (int i = 0; i < 8; ++i) {
        enum PersonRoles role;
        role = person->roles_mask & (0x01 << i);

        if (role > 0)
            StringArr_append(arr, person_role_to_str(role), -1);
    }

    return arr;
}