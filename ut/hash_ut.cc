#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "UnitTest++.h"

extern "C" {

#include "libpool.h"
#include "hash.h"
#include "libcache_def.h"

typedef struct test_data_t {
    void* key;
    void* entry;
} test_data_t;

uint32_t test_key_to_int(const void* key)
{
    uint32_t* value = (uint32_t*) key;
    return *value;
}

libcache_cmp_ret_t test_key_com(const void* key1, const void* key2)
{
    uint32_t* a = (uint32_t*) key1;
    uint32_t* b = (uint32_t*) key2;

    if (*a == *b) {
        return LIBCACHE_EQU;
    } else if (*a < *b) {
        return LIBCACHE_SMALLER;
    } else {
        return LIBCACHE_BIGER;
    }
}
}

struct HashFixture {
    hash_t* g_hash;
    list_t* list;
    void * pools;

    HashFixture()
    {
        const int max_entry = 655350;

        pool_attr_t pool_attr[] = {
                { 1, 1 },
                { 1, 1 },
                { sizeof(list_t), hash_get_bucket_count(max_entry)},
                { sizeof(node_t), max_entry},
                { 1, 1 },
                { 1, 1 },
                { sizeof(hash_t), 1 }, // POOL_TYPE_HASH_T
                { hash_calculate_bucket_size(max_entry), 1 }, // POOL_TYPE_BUCKET_T
                { sizeof(hash_data_t), max_entry },
                };

        const int pool_count = sizeof(pool_attr) / sizeof(pool_attr_t);
        size_t large_mem_size = pool_caculate_total_length(pool_count, pool_attr);

        void *large_memory = malloc(large_mem_size);
        assert(large_memory != NULL);
        pools = pools_init(large_memory, large_mem_size, pool_count, pool_attr);
        assert(pools != NULL);
        g_hash = (hash_t*) hash_init(max_entry, sizeof(int), test_key_com, test_key_to_int, pools);
        list = (list_t*) malloc(sizeof(list_t));
        list_init(list);
    }
    ~HashFixture()
    {
        node_t* list_entry = NULL;
        while (NULL != list && list_size(list) > 0 && (NULL != (list_entry = list_pop_front(list)))) {
            free(list_entry);
        }
        free(list);
        free(pools);
    }

    int init_hash_table()
    {
        node_t* list_entry = NULL;
        node_t* hash_entry = NULL;

        int i = 0;
        for (i = 0; i < 655350; i++) {
            list_entry = (node_t*) malloc(sizeof(node_t));
            list_entry->usr_data = (test_data_t*) malloc(sizeof(test_data_t));
            test_data_t* td = (test_data_t*) list_entry->usr_data;
            td->key = (int*) malloc(sizeof(int));
            memcpy(td->key, &i, sizeof(int));

            hash_entry = (node_t*) hash_add(g_hash, &i, list_entry, pools);
            if (hash_entry == NULL) {
                printf("insert to hash failed!\n");
                return -1;
            }
            td->entry = hash_entry;
            list_push_front(list, list_entry);
        }
        return 0;
    }

};

TEST_FIXTURE(HashFixture, TestAddHash)
{
    int ret = init_hash_table();
    CHECK(ret == 0);

    int count = hash_get_count(g_hash);
    CHECK(count == 655350);

    int i = 0;
    uint32_t sum = 0;
    for (i = 0; i <= g_hash->buckets_count; i++) {
        bucket_t bucket = g_hash->bucket_list[i];
        if (bucket.list != NULL) {
            sum += bucket.list_count;
        }
    }
    CHECK(count == 655350);

    hash_free(g_hash, pools);
    CHECK(g_hash->entry_count == 0);
    CHECK(g_hash->bucket_list[0].list == NULL);
    CHECK(g_hash->bucket_list[0].list_count == 0);
}

TEST_FIXTURE(HashFixture, TestFindHash)
{
    int ret = init_hash_table();
    CHECK(ret == 0);

    int value = 2000;
    node_t* node = (node_t*) hash_find(g_hash, &value);
    CHECK(node != NULL);

    int* p1 = (int*) (((test_data_t*) node->usr_data)->key);
    node_t* pp = (node_t*) (((test_data_t*) node->usr_data)->entry);
    hash_data_t* hd = (hash_data_t*) pp->usr_data;
    int* p2 = (int*) hd->key;

    CHECK(*p1 == 2000);
    CHECK(*p2 == 2000);

    int value2 = 655360;
    node = (node_t*) hash_find(g_hash, &value2);
    CHECK(node == NULL);
    hash_free(g_hash, pools);
}

TEST_FIXTURE(HashFixture, TestDelHash)
{
    int ret = init_hash_table();
    CHECK(ret == 0);

    //65549 is the first element in list
    int value = 655349;
    node_t* node = (node_t*) hash_find(g_hash, &value);
    CHECK(node != NULL);

    //get the first elements
    node_t* node2 = list_pop_front(list);
    test_data_t* td = (test_data_t*) node2->usr_data;

    ret = hash_del(g_hash, &value, td->entry, pools);
    CHECK(ret == 0);

    node = (node_t*) hash_find(g_hash, &value);
    CHECK(node == NULL);

    int count = hash_get_count(g_hash);
    CHECK(count == 655349);

    hash_destroy(g_hash, pools);
}

