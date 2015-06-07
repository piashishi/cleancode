#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "UnitTest++.h"
#include "list.h"
#include "libpool.h"

#define TEST_POOL_TYPE_DATA (0)
#define TEST_POOL_TYPE_2ND  (1)
#define TEST_POOL_TYPE_MAX  (2)

TEST(libpool_ut_init)
{

    size_t element_size = 4;
    const int entry_count = 25;

    pool_attr_t pool_attr[] = {{element_size, entry_count}};
    size_t large_mem_size = pool_caculate_total_length(POOL_TYPE_MAX, pool_attr);

    void *large_memory = malloc(large_mem_size);
    CHECK(large_memory != NULL);
    void *pools = pools_init(large_memory, large_mem_size - 1, POOL_TYPE_MAX, pool_attr);
    CHECK(pools == NULL);


    pools = pools_init(large_memory, large_mem_size, POOL_TYPE_MAX, pool_attr);
    CHECK(pools != NULL);

    free(pools);
}

TEST(libpool_ut_2_typse_of_pools)
{

    size_t element_size = 4;
    const int entry_count = 25;

    pool_attr_t pool_attr[] = {{element_size, entry_count}, {element_size, entry_count}};
    size_t large_mem_size = pool_caculate_total_length(TEST_POOL_TYPE_MAX, pool_attr);

    void *large_memory = malloc(large_mem_size);
    CHECK(large_memory != NULL);
    void *pools = pools_init(large_memory, large_mem_size, TEST_POOL_TYPE_MAX, pool_attr);
    CHECK(pools != NULL);

    void *entry = pool_get_element(pools, TEST_POOL_TYPE_DATA);
    CHECK(entry != NULL);

    entry = pool_get_element(pools, TEST_POOL_TYPE_2ND);
    CHECK(entry != NULL);

    free(pools);
}

TEST(libpool_ut_get_element)
{
    size_t mem_size = 100;
    size_t element_size = 4;
    const int entry_count = 25;

    pool_attr_t pool_attr[] = {{element_size, entry_count}};
    size_t large_mem_size = pool_caculate_total_length(POOL_TYPE_MAX, pool_attr);
    void* large_mem = malloc(large_mem_size);
    CHECK_EQUAL(!large_mem, 0);

    void *pools = pools_init(large_mem, large_mem_size, POOL_TYPE_MAX, pool_attr);
    CHECK_EQUAL(!pools, 0);

    void * entry_stack[entry_count];

    int i;
    void *entry;
    for (i = 0; i < entry_count; i++) {
        entry = pool_get_element(pools, POOL_TYPE_DATA);
        CHECK_EQUAL(entry != (void* )NULL, TRUE);

        // printf("entry[%d] = %X\n", i, entry);
        entry_stack[i] = entry;
    }

    // printf("entry[%d] = %X\n", i, entry);

    entry = pool_get_element(pools, POOL_TYPE_DATA);
    CHECK_EQUAL(entry == (void* )NULL, TRUE);

    return_t ret;
    for (i = 0; i < entry_count; i++) {
        ret = pool_free_element(pools, POOL_TYPE_DATA, entry_stack[i]);
        CHECK_EQUAL(ret, OK);
    }

    ret = pool_free_element(pools, POOL_TYPE_DATA, entry_stack[i]);
    CHECK_EQUAL(ret, ERR);

    free(pools);
}

