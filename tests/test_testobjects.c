#include "testrunner.h"
#include "testobjects.h"
#include "arena.h"

TEST_SETUP(persons_suite)

static testdata_Persons persons;
static testdata_Person* pers[5] = {0};
static mem_Arena arena;

static void add_person(
    size_t idx, const char *name, const char *email,
    enum testdata_person_Roles roles
) {
    testdata_Person *person = mem_arena_alloc(&arena, sizeof(testdata_Person));
    testdata_person_init(person);

    types_string_push_str(&person->name, name, strlen(name), &arena);
    types_string_push_str(&person->email, email, strlen(email), &arena);
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
    testdata_Person person;
    testdata_person_init(&person);

    expectEQ((void*)person.email.data, NULL);
    expectEQ((void*)person.name.data, NULL);
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
    testdata_persons_init(&persons);
}

TEST(persons_suite, persons_init, "Should init")
{
    expectEQ(persons.len, 0);
    expectEQ(persons.size, 0);
    expectEQ((void*)persons.data, NULL);
}

TEST(persons_suite, prealloc, "Should preallocate")
{
    testdata_persons_pre_alloc(&persons, 5, &arena);
    expectEQ(persons.len, 0);
    expectEQ(persons.size, 5);
    expectNE((void*)persons.data, NULL);
}

TEST(persons_suite, addperson, "Should add a person")
{
    add_person(0, "TestName", "text@fake.nu", Teacher);
    testdata_persons_add_person(&persons, pers[0], &arena);
    expectEQ(persons.len, 1);
    expectEQ(persons.size, 1);
    expectEQ((void*)persons.data[0], pers[0]);
    expectEQ(persons.data[0]->name.data, "TestName");
    expectEQ(persons.data[0]->email.data, "text@fake.nu");


    add_person(1, "Name2", "test2@morefake.com", Student);
    testdata_persons_add_person(&persons, pers[1], &arena);
    expectEQ(persons.len, 2);
    expectEQ(persons.size, 2);
    expectEQ((void*)persons.data[1], pers[1]);
    expectEQ(persons.data[1]->name.data, "Name2");
    expectEQ(persons.data[1]->email.data, "test2@morefake.com");
}

TEST(persons_suite, indexof, "Should return indexof")
{
    add_person(0, "TestName", "text@fake.nu", Teacher);
    add_person(1, "Name2", "test2@morefake.com", Student);
    int32_t idx = testdata_persons_index_of(&persons, pers[0]);
    expectEQ(idx, -1);

    testdata_persons_add_person(&persons, pers[0], &arena);
    testdata_persons_add_person(&persons, pers[1], &arena);

    idx = testdata_persons_index_of(&persons, pers[0]);
    expectEQ(idx, 0);

    idx = testdata_persons_index_of(&persons, pers[1]);
    expectEQ(idx, 1);
}

TEST(persons_suite, insert, "Should insert")
{
    add_person(0, "TestName", "text@fake.nu", Teacher);
    add_person(1, "Name2", "test2@morefake.com", Student);
    add_person(2, "Name3", "test3@faker.com", External);
    add_person(3, "Name4", "test4@faking.com", External);

    int32_t idx = testdata_persons_insert(&persons, pers[0], pers[1], &arena);
    expectEQ(idx, 0);

    idx = testdata_persons_insert(&persons, pers[1], pers[0], &arena);
    expectEQ(idx, 0);

    idx = testdata_persons_insert(&persons, pers[2], pers[0], &arena);
    expectEQ(idx, 1);

    idx = testdata_persons_insert(&persons, pers[2], NULL, &arena);
    expectEQ(idx, 3);
}

TEST(persons_suite, pers_remove, "Should remove")
{
    bool res = testdata_persons_remove(&persons, pers[0]);
    expectFalse(res);

    add_person(0, "TestName", "text@fake.nu", Teacher);
    add_person(1, "Name2", "test2@morefake.com", Student);
    add_person(2, "Name3", "test3@faker.com", External);
    add_person(3, "Name4", "test4@faking.com", External);
    testdata_persons_add_person(&persons, pers[0], &arena);
    testdata_persons_add_person(&persons, pers[1], &arena);
    testdata_persons_add_person(&persons, pers[2], &arena);
    testdata_persons_add_person(&persons, pers[3], &arena);


    res = testdata_persons_remove(&persons, pers[0]);
    expectTrue(res);
    res = testdata_persons_remove(&persons, pers[0]);
    expectFalse(res);

    res = testdata_persons_remove(&persons, pers[2]);
    expectTrue(res);
    res = testdata_persons_remove(&persons, pers[2]);
    expectFalse(res);

    res = testdata_persons_remove(&persons, pers[3]);
    expectTrue(res);
    res = testdata_persons_remove(&persons, pers[3]);
    expectFalse(res);


    res = testdata_persons_remove(&persons, pers[1]);
    expectTrue(res);
    res = testdata_persons_remove(&persons, pers[1]);
    expectFalse(res);
}

// -----------------------------------------------------

TEST_SETUP(obj_suite)

TEST_SUITE_SETUP_FN(obj_suite)
{
    mem_arena_init(&arena);
}

TEST_SUITE_TEARDOWN_FN(obj_suite)
{
    mem_arena_free(&arena);
}

TEST(obj_suite, objinit, "Should init")
{
    testdata_Obj obj;
    testdata_obj_init(&obj);
    expectEQ(obj.flags, 0);
    expectEQ(obj.type, 0);
    expectEQ((void*)obj.string.data, NULL);
    expectEQ(obj.string.len, 0);
}
