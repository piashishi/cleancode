#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "UnitTest++.h"

extern "C" {

#include "hash.h"
#include "libcache_def.h"

typedef struct test_data_t
{
    void* key;
    void* entry;
}test_data_t;

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
    HashFixture()
    {
        g_hash = NULL;
        g_hash = (hash_t*)hash_init(sizeof(int), test_key_com, test_key_to_int);
    }
    ~HashFixture()
    {

    }
    list_t* list;

    int init_hash_table()
    {
        list = (list_t*) malloc(sizeof(list_t));
        list_init(list);

        node_t* list_entry = NULL;
        node_t* hash_entry = NULL;

        int i = 0;
        for (i = 0; i < 655350; i++) {
            list_entry = (node_t*) malloc(sizeof(node_t));
            list_entry->usr_data = (test_data_t*)malloc(sizeof(test_data_t));
            test_data_t* td = (test_data_t*)list_entry->usr_data;
            td->key = (int*) malloc(sizeof(int));
            memcpy(td->key, &i, sizeof(int));

            hash_entry = (node_t*) hash_add(g_hash, &i, list_entry);
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
    for (i = 0; i <= MAX_BUCKETS; i++) {
        bucket_t bucket = g_hash->bucket_list[i];
        if (bucket.list != NULL) {
            sum += bucket.list_count;
        }
    }
    CHECK(count == 655350);

    hash_free(g_hash);
    CHECK(g_hash->entry_count == 0);
    CHECK(g_hash->bucket_list[0].list == NULL);
    CHECK(g_hash->bucket_list[0].list_count  == 0);
}

TEST_FIXTURE(HashFixture, TestFindHash)
{
    int ret = init_hash_table();
    CHECK(ret == 0);

    int value = 2000;
    node_t* node = (node_t*) hash_find(g_hash, &value);
    CHECK(node != NULL);

    int* p1 = (int*) (((test_data_t*)node->usr_data)->key);
    node_t* pp = (node_t*)(((test_data_t*)node->usr_data)->entry);
    hash_data_t* hd = (hash_data_t*)pp->usr_data;
    int* p2 = (int*) hd->key;

    CHECK(*p1 == 2000);
    CHECK(*p2 == 2000);

    int value2 = 655360;
    node = (node_t*) hash_find(g_hash, &value2);
    CHECK(node == NULL);
    hash_free(g_hash);
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
    test_data_t* td = (test_data_t*)node2->usr_data;

    ret = hash_del(g_hash, &value, td->entry);
    CHECK(ret == 0);

    node = (node_t*) hash_find(g_hash, &value);
    CHECK(node == NULL);

    int count = hash_get_count(g_hash);
    CHECK(count == 655349);
    
    hash_destroy(g_hash);
}

