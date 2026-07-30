#ifndef _TEST_PERSON_H_
#define _TEST_PERSON_H_

#include <stdint.h>
#include "typestring.h"

/**
 * The roles a person can have
 */
enum PersonRoles {
    PersonRoleUndefined,
    TeacherRole = 0x01,
    StudentRole = 0x02,
    ExternalRole = 0x04,
    OtherRole    = 0x08,
    _PersonRolesEndMarker
};

/**
 * A single person
 */
typedef struct Person
{
    uint32_t roles_mask; ///< The doc_person_Roles this person has
    String name; ///< the name of this person.
    String email; ///< the email of this person.
} Person;


const char* person_role_to_str(enum PersonRoles role);

enum PersonRoles person_str_to_role(const char* role_str);

/**
 * An array of persons
 */
#ifndef NO_TEMPLATE_PERSON

#define T Person
#define NAME PersonArr
#include "array.template.h"

#endif


/**
 * Initialize a person
 */
void Person_init(Person *person, mem_Arena* arena);

/**
 * Get this persons roles as string
 */
StringArr* Person_roles(Person *person, mem_Arena* arena);



#endif // _TEST_PERSON_H_
