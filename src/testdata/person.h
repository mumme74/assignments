#ifndef _TEST_PERSON_H_
#define _TEST_PERSON_H_

#include <stdint.h>
#include "typestring.h"

/**
 * The roles a person can have
 */
enum PersonRoles {
    Teacher = 0x01,
    Student = 0x02,
    External = 0x04,
    Other    = 0x08
};

/**
 * A single person
 */
typedef struct Person
{
    uint8_t roles_mask; ///< The doc_person_Roles this person has
    String name; ///< the name of this person.
    String email; ///< the email of this person.
} Person;

/**
 * An array of persons
 */
#ifndef NO_TEMPLATE_PERSON

#define T Person
#define NAME PersonArr
#include "array.template.h"

#endif


void Person_init(Person *person, mem_Arena* arena);

#endif // _TEST_PERSON_H_