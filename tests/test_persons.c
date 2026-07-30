#include "testrunner.h"

#include "person.h"
#include "arena.h"


TEST_SETUP(persons_suite)

static PersonArr persons;
static Person* pers[5] = {0};
static mem_Arena arena;

static void add_person(
    size_t idx, const char *name, const char *email,
    enum PersonRoles roles
) {
    Person *person = mem_arena_alloc(&arena, sizeof(Person));
    Person_init(person, &arena);

    String_append_str(&person->name, name, strlen(name));
    String_append_str(&person->email, email, strlen(email));
    person->roles_mask |= roles;

    pers[idx] = person;

}

static void clear_pers()
{
    memset(pers, 0, sizeof(*pers));
}

// ------------------------------------------

TEST_SETUP(pers_suite)

TEST_SETUP_FN(pers_suite)
{
    clear_pers();
}

TEST(pers_suite, pers_init, "Should init pers")
{
    Person person;
    Person_init(&person, &arena);

    expectEQ((void*)person.email.elements, NULL);
    expectEQ((void*)person.name.elements, NULL);
    expectEQ(person.roles_mask, 0);
}


// -----------------------------------------

TEST_SUITE_SETUP_FN(persons_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(persons_suite)
{
    mem_arena_free(&arena);
}

TEST_SETUP_FN(persons_suite)
{
    PersonArr_init(&persons, &arena);
}


TEST(persons_suite, addperson, "Should add a person")
{
    add_person(0, "TestName", "text@fake.nu", TeacherRole);
    PersonArr_push_back(&persons, *pers[0]);
    expectEQ(persons.size, 1);
    expectEQ(persons.elements[0].name.elements, "TestName");
    expectEQ(persons.elements[0].email.elements, "text@fake.nu");


    add_person(1, "Name2", "test2@morefake.com", StudentRole);
    PersonArr_push_back(&persons, *pers[1]);
    expectEQ(persons.size, 2);
    expectEQ(persons.elements[1].name.elements, "Name2");
    expectEQ(persons.elements[1].email.elements, "test2@morefake.com");
}

TEST(persons_suite, indexof, "Should return indexof")
{
    add_person(0, "TestName", "text@fake.nu", TeacherRole);
    add_person(1, "Name2", "test2@morefake.com", StudentRole);
    int32_t idx = PersonArr_index_of(&persons, *pers[0]);
    expectEQ(idx, -1);

    PersonArr_push_back(&persons, *pers[0]);
    PersonArr_push_back(&persons, *pers[1]);

    idx = PersonArr_index_of(&persons, *pers[0]);
    expectEQ(idx, 0);

    idx = PersonArr_index_of(&persons, *pers[1]);
    expectEQ(idx, 1);
}

TEST(persons_suite, insert, "Should insert")
{
    add_person(0, "TestName", "text@fake.nu", TeacherRole);
    add_person(1, "Name2", "test2@morefake.com", StudentRole);
    add_person(2, "Name3", "test3@faker.com", ExternalRole);
    add_person(3, "Name4", "test4@faking.com", ExternalRole);
    PersonArr_push_back(&persons, *pers[0]);
    PersonArr_push_back(&persons, *pers[1]);
    PersonArr_push_back(&persons, *pers[2]);
    PersonArr_push_back(&persons, *pers[3]);

    expectTrue(PersonArr_insert(&persons, *pers[0], 1));

    expectTrue(PersonArr_insert(&persons, *pers[1], 0));

    expectTrue(PersonArr_insert(&persons, *pers[2], 0));
}

TEST(persons_suite, pers_remove, "Should remove")
{
    expectFalse(PersonArr_remove(&persons, 0));

    add_person(0, "TestName", "text@fake.nu", TeacherRole);
    add_person(1, "Name2", "test2@morefake.com", StudentRole);
    add_person(2, "Name3", "test3@faker.com", ExternalRole);
    add_person(3, "Name4", "test4@faking.com", ExternalRole);
    PersonArr_push_back(&persons, *pers[0]);
    PersonArr_push_back(&persons, *pers[1]);
    PersonArr_push_back(&persons, *pers[2]);
    PersonArr_push_back(&persons, *pers[3]);


    expectTrue(PersonArr_remove(&persons, 0));
    expectFalse(PersonArr_remove(&persons, 3));

    expectTrue(PersonArr_remove(&persons, 2));
    expectFalse(PersonArr_remove(&persons, 2));

    expectTrue(PersonArr_remove(&persons, 1));
    expectFalse(PersonArr_remove(&persons, 1));

    expectTrue(PersonArr_remove(&persons, 0));
    expectFalse(PersonArr_remove(&persons, 0));
}
