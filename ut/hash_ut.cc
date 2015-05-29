#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "UnitTest++.h"

extern "C" {

#include "hash.h"

typedef struct test_data_t
{
    void* key;
    void* entry;
}test_data_t;

u32 test_key_to_int(const void* key)
{
    u32* value = (u32*) key;
    return *value;
}

int test_key_com(const void* key1, const void* key2)
{
    u32* a = (u32*) key1;
    u32* b = (u32*) key2;

    if (*a == *b) {
        return 0;
    } else if (*a < *b) {
        return -1;
    } else {
        return 1;
    }
}
}

struct HashFixture {
    hash_t* g_hash;
    HashFixture()
    {
        g_hash = hash_init(sizeof(int), test_key_com, test_key_to_int);
    }
    ~HashFixture()
    {

    }
    list_t* list;

    int init_hash_table()
    {
        list = (list_t*) malloc(sizeof(list_t));
        node_t* list_entry = NULL;
        node_t* hash_entry = NULL;
        list_init(list);

        int i = 0;
        for (i = 0; i < 655350; i++) {
            list_entry = (node_t*) malloc(sizeof(node_t));
            list_entry->usr_data = (test_data_t*)malloc(sizeof(test_data_t));
            list_entry->usr_data->key = (int*) malloc(sizeof(int));
            memcpy(list_entry->usr_data->key, &i, sizeof(int));

            hash_entry = (node_t*) hash_add(g_hash, &i, list_entry);
            if (hash_entry == NULL) {
                printf("insert to hash failed!\n");
                return -1;
            }
            list_entry->usr_data->entry = hash_entry;
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
    u32 sum = 0;
    for (i = 0; i <= MAX_BUCKETS; i++) {
        bucket_t bucket = g_hash->bucket_list[i];
        if (bucket.list != NULL) {
            sum += bucket.list_count;
        }
    }
    CHECK(count == 655350);
}

TEST_FIXTURE(HashFixture, TestFindHash)
{
    int ret = init_hash_table();
    CHECK(ret == 0);

    int value = 2000;
    node_t* node = (node_t*) hash_find(g_hash, &value);
    CHECK(node != NULL);
    int* p1 = (int*) (((test_data_t*)node->usr_data)->key);
    node_t* pp = (node_t*) ((test_data_t*)node->usr_data)->entry);
    int* p2 = (int*) pp->key;
    CHECK(*p1 == 2000);
    CHECK(*p2 == 2000);

    int value2 = 655360;
    node = (node_t*) hash_find(g_hash, &value2);
    CHECK(node == NULL);
}

TEST_FIXTURE(HashFixture, TestDelHash)
{
    int ret = init_hash_table();
    CHECK(ret == 0);

    int value = 655349;
    node_t* node = (node_t*) hash_find(g_hash, &value);
    hash_data_t* hd = node->usr_data;
    CHECK(node != NULL);

    node = list_pop_front(list);
    ret = hash_del(g_hash, hd->key, hd->cache_node_ptr);
    CHECK(ret == 0);

    node = (node_t*) hash_find(g_hash, &value);
    CHECK(node == NULL);

    int count = hash_get_count();
    CHECK(count == 655349);
}

