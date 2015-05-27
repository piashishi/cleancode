#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "UnitTest++.h"
#include "list.h"
#include "libpool.h"

extern element_pool_t pool;

TEST(libpool_ut_init)
{
    int mem_size = 100;
    int element_size = 4;
    const int entry_count = 25;

    int ret = pool_init(mem_size);
    CHECK_EQUAL(ret, OK);

    ret = pool_init_element_pool(element_size, entry_count);
    CHECK_EQUAL(ret, OK);

    mem_size = 125;
    element_size = 5;

    ret = pool_init(mem_size);
    CHECK_EQUAL(ret, OK);

    ret = pool_init_element_pool(element_size, entry_count);
    CHECK_EQUAL(ret, ERR);
}

TEST(libpool_ut_get_element)
{
    int mem_size = 100;
    int element_size = 4;
    const int entry_count = 25;

    int ret = pool_init(mem_size);
    CHECK_EQUAL(ret, OK);

    ret = pool_init_element_pool(element_size, entry_count);
    CHECK_EQUAL(ret, OK);

    void * entry_stack[entry_count];

    int i;
    void *entry;
    for (i = 0; i < entry_count; i++) {
        entry = pool_get_element();
        CHECK_EQUAL(entry != (void* )NULL, TRUE);
        entry_stack[i] = entry;
    }

    entry = pool_get_element();
    CHECK_EQUAL(entry == (void* )NULL, TRUE);

    for (i = 0; i < entry_count; i++) {
        ret = pool_free_element(entry_stack[i]);
        CHECK_EQUAL(ret, OK);
    }

    ret = pool_free_element(entry_stack[i]);
    CHECK_EQUAL(ret, ERR);

}

