#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "UnitTest++.h"
#include "list.h"
#include "libpool.h"

extern element_pool_t pool;

TEST(libpool_ut)
{
    int mem_size = 100;
    int element_size = 4;
    const int entry_count = 25;

    void* entry_temp[entry_count];

    int ret = pool_init(mem_size);
    CHECK_EQUAL(ret, OK);

    ret = pool_init_element_pool(element_size, entry_count);
    CHECK_EQUAL(ret, OK);

//    printf("pool.start_memory = %X\n", pool.start_memory);
//    int i;
//    for (i = 0; i < entry_count + 1; i++) {
//        void *entry = (node_t *)pool_get_element();
//        memcpy(&entry_temp[i], &entry, sizeof(void *));
//        printf("entry_temp[%d] = %X \n", i, entry);
//        printf("entry     [%d] = %X \n\n", i, entry);
//    }
//
//    printf("\n");
//
//    for (i = 0; i < entry_count; i++) {
//        printf("element_temp[%d] = %X \n", i, entry_temp[i]);
//        node_t* node = get_real_node_addr(entry_temp[i]);
//        printf("node[%d]         = %X \n\n", i, node);
//        CHECK_EQUAL(pool_free_element(entry_temp[i]), OK);
//    }
//
//
//    printf("\n");
//
//    for (i = 0; i < entry_count; i++) {
//        void *entry = (node_t *)pool_get_element();
//        memcpy(&entry_temp[i], &entry, sizeof(void *));
//        printf("entry_temp[%d] = %X \n", i, entry);
//        printf("entry     [%d] = %X \n\n", i, entry);
//    }
}
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
        CHECK_EQUAL(entry != (void*)NULL, TRUE);
        entry_stack[i] = entry;
    }

    entry = pool_get_element();
    CHECK_EQUAL(entry == (void*)NULL, TRUE);

    for (i = 0; i < entry_count; i++) {
        ret = pool_free_element(entry_stack[i]);
        CHECK_EQUAL(ret , OK);
    }

//    ret = pool_free_element(entry_stack[i]);
//    CHECK_EQUAL(ret , ERR);

}

