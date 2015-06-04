#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "UnitTest++.h"


extern "C" {

#include "libcache.h"
#include "libcache_def.h"

extern uint32_t test_key_to_int(const void* key);
extern  libcache_cmp_ret_t test_key_com(const void* key1, const void* key2);

typedef struct test_data_t
{
    void* key;
    void* entry;
}test_data_t;

}

struct LibCacheFixture {
    void* g_cache;
    LibCacheFixture()
    {
        g_cache = libcache_create(100, sizeof(int), sizeof(int), malloc, free, NULL, test_key_com, test_key_to_int);
    }
    ~LibCacheFixture()
    {

    }
};

TEST_FIXTURE(LibCacheFixture, TestAdd)
{
    int key = 100;
    int entry = 1000;
    int* value = (int*)libcache_add(g_cache, &key, &entry);
    CHECK(*value == entry);

    //check add same key/entry
    int* value2 = (int*) libcache_add(g_cache, &key, &entry);
    CHECK(value2 == NULL);

    //check add NULL entry
    int key3 = 300;
    int* value3 = (int*) libcache_add(g_cache, &key3, NULL);
    CHECK(value3 != NULL);
    *value3 = 3000;

    //check unlock
    libcache_ret_t ret = libcache_unlock_entry(g_cache, value3);
    CHECK(ret == LIBCACHE_SUCCESS);

    //check look up
    int entry4 = 0;
    int* value4 = (int*)libcache_lookup(g_cache, &key3, &entry4);
    CHECK(entry4 == 3000);
    CHECK(*value4 == 3000);

    //check look up
    int* value5 = (int*)libcache_lookup(g_cache, &key3, NULL);
    CHECK(value5 == value3);


    ret = libcache_clean(g_cache);
    CHECK(ret == LIBCACHE_SUCCESS);
}



TEST_FIXTURE(LibCacheFixture, TestLookup)
{
    int i = 0;
    int* key = NULL;
    int* entry = NULL;
    int* entryList[100] = {0};
    for(i = 0; i<100;i++)
    {
        key = (int*)malloc(sizeof(int));
        entry = (int*)malloc(sizeof(int));
        *key = i;
        *entry = 100*i;
        int* value = (int*)libcache_add(g_cache, key, entry);

        int* ptr = (int*)libcache_lookup(g_cache, key, NULL);
        CHECK(*ptr == *entry);
        entryList[i] = ptr; 
    }

   int key2 = 300;
   int entry2 = 3000;
    
   //check all node were locked, no anymore node.
   int* value2 = (int*)libcache_add(g_cache, &key2, &entry2);
   CHECK(value2 == NULL);

   libcache_ret_t ret = LIBCACHE_SUCCESS;
   for(i = 0; i<100;i++)
   {
        ret = libcache_unlock_entry(g_cache, entryList[i]);
        CHECK(ret == LIBCACHE_SUCCESS);
        ret = libcache_unlock_entry(g_cache, entryList[i]);
        CHECK(ret == LIBCACHE_UNLOCKED);
   }
   value2 = (int*)libcache_add(g_cache, &key2, &entry2);
   CHECK(*value2 == entry2);

   ret = libcache_clean(g_cache);
   CHECK(ret == LIBCACHE_SUCCESS);
}

TEST_FIXTURE(LibCacheFixture, TestDelete)
{
    int key  = 1;
    int entry = 100;

    int key2 = 2;
    int entry2 = 200;

    int key3 = 3;
    int entry3 = 300;

    int* value = (int*)libcache_add(g_cache, &key, &entry);
    int* value2 = (int*)libcache_add(g_cache, &key2, &entry2);
    int* value3= (int*)libcache_add(g_cache, &key3, &entry3);
 
    libcache_scale_t  count = libcache_get_entry_number(g_cache);
    CHECK(count == 3);

    libcache_ret_t ret = libcache_delete_by_key(g_cache, &key);
    CHECK(ret == LIBCACHE_SUCCESS);
    count = libcache_get_entry_number(g_cache);
    CHECK(count == 2);

    ret = libcache_delete_by_key(g_cache, &key);
    CHECK(ret == LIBCACHE_NOT_FOUND);
    count = libcache_get_entry_number(g_cache);
    CHECK(count == 2);

    ret = libcache_delete_entry(g_cache, &entry2);
    CHECK(ret == LIBCACHE_SUCCESS);
    count = libcache_get_entry_number(g_cache);
    CHECK(count == 1);

    ret = libcache_delete_entry(g_cache, &entry2);
    count = libcache_get_entry_number(g_cache);
    CHECK(ret == LIBCACHE_NOT_FOUND);
    CHECK(count == 1);

    libcache_lookup(g_cache, &key3, NULL);
    ret = libcache_delete_by_key(g_cache, &key3);
    CHECK(ret == LIBCACHE_LOCKED);
    count = libcache_get_entry_number(g_cache);
    CHECK(count == 1);

    ret = libcache_delete_entry(g_cache, &entry3);
    CHECK(ret == LIBCACHE_LOCKED);
    count = libcache_get_entry_number(g_cache);
    CHECK(count == 1);

    ret = libcache_clean(g_cache);
    CHECK(ret == LIBCACHE_SUCCESS);
}

TEST_FIXTURE(LibCacheFixture, TestSwap)
{
    int* key = NULL;
    int* entry = NULL;
    
    int i = 0;
    for(i = 0; i<100;i++)
    {
        key = (int*)malloc(sizeof(int));
        entry = (int*)malloc(sizeof(int));
        *key = i;
        *entry = 100*i;
        int* value = (int*)libcache_add(g_cache, key, entry);
        CHECK(*value = *entry);
    }
    int key2 = 200;
    int entry2 = 2000;
    int* value2 = (int*)libcache_add(g_cache, &key2, &entry2);
    CHECK(value2 != NULL);
    CHECK(*value2 = entry2);

    //the first entry should be swap out
    int key3 = 0;
    int entry3 = 0;
    int* value3 = (int*)libcache_lookup(g_cache, &key3, &entry3);
    CHECK(value3 == NULL);

    libcache_destroy(g_cache);

    g_cache = libcache_create(6553500, sizeof(int), sizeof(int), malloc, free, NULL, test_key_com, test_key_to_int);

    libcache_scale_t  count = libcache_get_max_entry_number(g_cache);
    CHECK(count == 6553500);

    for(i = 0; i<65535000;i++)
    {
        int* value4 = (int*)libcache_add(g_cache, &i, &i);
        CHECK(*value4 = i);
    }
    count = libcache_get_entry_number(g_cache);
    CHECK(count == 6553500);

    for(i = 0; i<65535;i++)
    {
        int entry5 = 0;
        int* value5 = (int*)libcache_lookup(g_cache, &i, &entry5);
        CHECK(value5 == NULL);
    }
}
