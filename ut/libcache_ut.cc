#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "UnitTest++.h"


extern "C" {

#include "libcache.h"
#include "libcache_def.h"

static	uint32_t test_key_to_int(const void* key)
	{
		uint32_t* value = (uint32_t*) key;
		return *value;
	}

static libcache_cmp_ret_t test_key_com(const void* key1, const void* key2)
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


const libcache_scale_t g_max_entry_number = 100;

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
        g_cache = libcache_create(g_max_entry_number, sizeof(int), sizeof(int), malloc, free, NULL, test_key_com, test_key_to_int);
    }
    ~LibCacheFixture()
    {
        libcache_destroy(g_cache);
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
    int* entryList[g_max_entry_number] = { 0 };
    for (i = 0; i <= g_max_entry_number; i++) {
        key = (int*) malloc(sizeof(int));
        entry = (int*) malloc(sizeof(int));
        *key = i;
        *entry = 100 * i;
        int* value = (int*) libcache_add(g_cache, key, entry);

        int* ptr = (int*) libcache_lookup(g_cache, key, NULL);
        CHECK_EQUAL(*ptr, *entry);
        entryList[i] = ptr;
    }

    int key2 = 300;
    int entry2 = 3000;

    //check all node were locked, no anymore node.
    int* value2 = (int*) libcache_add(g_cache, &key2, &entry2);
    CHECK(value2 == NULL);

    libcache_ret_t ret = LIBCACHE_SUCCESS;
    for (i = 0; i <= g_max_entry_number; i++) {
        ret = libcache_unlock_entry(g_cache, entryList[i]);
        CHECK_EQUAL(ret, LIBCACHE_SUCCESS);
        ret = libcache_unlock_entry(g_cache, entryList[i]);
        CHECK_EQUAL(ret, LIBCACHE_UNLOCKED);
    }
    value2 = (int*) libcache_add(g_cache, &key2, &entry2);
    CHECK_EQUAL(*value2, entry2);

    ret = libcache_clean(g_cache);
    CHECK_EQUAL(ret, LIBCACHE_SUCCESS);
}

TEST_FIXTURE(LibCacheFixture, TestDelete)
{
    int key = 1;
    int entry = 100;

    int key2 = 2;
    int entry2 = 200;

    int key3 = 3;
    int entry3 = 300;

    int* value = (int*) libcache_add(g_cache, &key, &entry);
    int* value2 = (int*) libcache_add(g_cache, &key2, &entry2);
    int* value3 = (int*) libcache_add(g_cache, &key3, &entry3);

    libcache_scale_t count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(count, 3);

    libcache_ret_t ret = libcache_delete_by_key(g_cache, &key);
    CHECK_EQUAL(ret, LIBCACHE_SUCCESS);
    count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(count, 2);

    ret = libcache_delete_by_key(g_cache, &key);
    CHECK_EQUAL(ret, LIBCACHE_NOT_FOUND);
    count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(count, 2);

    ret = libcache_delete_entry(g_cache, value2); //TODO: can be entry2 ?
    CHECK_EQUAL(ret, LIBCACHE_SUCCESS);
    count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(count, 1);

    ret = libcache_delete_entry(g_cache, value2); //TODO: can be entry2 ?
    count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(ret, LIBCACHE_NOT_FOUND);
    CHECK_EQUAL(count, 1);

    libcache_lookup(g_cache, &key3, NULL);
    ret = libcache_delete_by_key(g_cache, &key3);
    CHECK_EQUAL(ret, LIBCACHE_LOCKED);
    count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(count, 1);

    ret = libcache_delete_entry(g_cache, value3); //TODO: can be entry3 ?
    CHECK_EQUAL(ret, LIBCACHE_LOCKED);
    count = libcache_get_entry_number(g_cache);
    CHECK_EQUAL(count, 1);

    ret = libcache_clean(g_cache);
    CHECK_EQUAL(ret, LIBCACHE_SUCCESS);
}

TEST_FIXTURE(LibCacheFixture, TestSwap)
{
    int* key = NULL;
    int* entry = NULL;

    int i = 0;
    for (i = 0; i <= g_max_entry_number; i++) {
        key = (int*) malloc(sizeof(int));
        entry = (int*) malloc(sizeof(int));
        *key = i;
        *entry = 100 * i;
        int* value = (int*) libcache_add(g_cache, key, entry);
        CHECK_EQUAL(*value, *entry);
    }

    int key2 = 200;
    int entry2 = 2000;
    int* value2 = (int*) libcache_add(g_cache, &key2, &entry2);
    CHECK(value2 != NULL);
    CHECK_EQUAL(*value2, entry2);

    //the first entry should be swap out
    int key3 = 0;
    int entry3 = 0;
    int* value3 = (int*) libcache_lookup(g_cache, &key3, &entry3);
    CHECK(value3 == NULL);

    libcache_destroy(g_cache);

    const libcache_scale_t max_entry_number = 65535;

    g_cache = libcache_create(max_entry_number, sizeof(int), sizeof(int), malloc, free, NULL, test_key_com, test_key_to_int);

	for (i = 0; i < max_entry_number * 10; i++) {
		int* value4 = (int*) libcache_add(g_cache, &i, &i);
		CHECK_EQUAL(*value4, i);
	}
	int count = libcache_get_entry_number(g_cache);
	CHECK_EQUAL(count, max_entry_number+1);

	for (i = 0; i <max_entry_number ; i++) {
		int entry5 = 0;
		int* value5 = (int*) libcache_lookup(g_cache, &i, &entry5);
		CHECK(value5 == NULL);
	}
}
