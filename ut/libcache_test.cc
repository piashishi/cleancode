/*
 * libcache_test.cc
 *
 *  Created on: Mar 24, 2015
 *      Author: root
 */

#include <time.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "UnitTest++.h"
#include "TestReporter.h"
#include "TestDetails.h"
#include "libpete.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "libcache.h"


#ifdef __cplusplus
}
#endif

#define MAX_IMSI_LEN 8
#define TEST_FAILURE 1
#define TEST_SUCCESS 0



typedef union {
    uint8_t  imsi[MAX_IMSI_LEN];
    uint64_t imsi64bit;
} imsi_t;

typedef struct {
#if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t      pres  : 1; /**< presence bit */
    uint8_t      bogus : 1; /**< bogus IMSI, used only inside application (TRUE/FALSE) */
    uint8_t      spare : 2; /**< spare bits for future use */
    uint8_t      len   : 4; /**< length field, max len of IMSI is MAX_IMSI_LEN */
#elif BYTE_ORDER == BIG_ENDIAN
    uint8_t      len   : 4; /**< length field, max len of IMSI is MAX_IMSI_LEN */
    uint8_t      spare : 2; /**< spare bits for future use */
    uint8_t      bogus : 1; /**< bogus IMSI, used only inside application (TRUE/FALSE) */
    uint8_t      pres  : 1; /**< presence bit */
#endif
    imsi_t  val;
} ng_imsi_t;

typedef struct {
    ng_imsi_t imsi;
    uint8_t service_group;
}cache_key_t;

typedef struct {
    uint32_t teid;       /* teid of f-teid */
    uint32_t    addr_bytes; /* address bytes from f-teid. ipv4 address or 4 last bytes of ipv6 */
} fteid_data_t;

typedef struct {
    time_t time_stamp;                       /* time stamp (seconds) */
    union {
        imsi_t imsi;
        fteid_data_t fteid;
    } data;
    uint8_t sgroup_id; /* service group id */
    uint8_t selected_gw;        /* number of selected gateway service */
    uint8_t data_type;                            /* data type: LIBLB_DATA_IMSI/LIBLB_DATA_FTEID */
    uint64_t value;
} liblb_cache_entry_t;

static libcache_cmp_ret_t cmp_key_imp(const void *key1, const void *key2)
{
    if (((cache_key_t *) key1)->imsi.val.imsi64bit
            == ((cache_key_t *) key2)->imsi.val.imsi64bit )
        return LIBCACHE_EQU;
    else
        return LIBCACHE_NOT_EQU;
}

static libcache_scale_t
key_to_number_imp(const void* key)
{
    uint64_t init_num = 0xE28D709B;
    init_num ^= ( ((cache_key_t *) key)->imsi.val.imsi64bit
            ^ (((cache_key_t *) key)->imsi.val.imsi64bit >> 32) );
    return init_num;
}

static int
libcache_test_insertion(void * libcache)
{
    cache_key_t key;
    key.imsi.val.imsi64bit = 0;
    libcache_scale_t max_entry_number = libcache_get_max_entry_number(libcache);
    while (1)
    {
        liblb_cache_entry_t* entry = (liblb_cache_entry_t*)libcache_lookup(libcache, (void*)&key, NULL);
        liblb_cache_entry_t* entry_exp;
        if (entry != NULL)
        {
            printf("libcache_lookup ERROR! %"PRIu64"\n", key.imsi.val.imsi64bit);
            return TEST_FAILURE;
        }
        entry = (liblb_cache_entry_t*)libcache_add(libcache, &key, NULL);
        if (entry == NULL)
        {
            printf("lib_cache_add ERROR! %"PRIu64"\n", key.imsi.val.imsi64bit);
            return TEST_FAILURE;
        }

        entry_exp = (liblb_cache_entry_t*)libcache_add(libcache, &key, NULL);
        if (entry_exp != NULL)
        {
            printf("lib_cache_add ERROR! %"PRIu64"\n", key.imsi.val.imsi64bit);
            return TEST_FAILURE;
        }

        /*Set user data for validation*/
        entry->data.imsi.imsi64bit = key.imsi.val.imsi64bit;
        entry->value = ((key.imsi.val.imsi64bit <<  32) | (key.imsi.val.imsi64bit >> 32));
        libcache_unlock_entry(libcache, entry);
        key.imsi.val.imsi64bit++;
        if(key.imsi.val.imsi64bit > max_entry_number)
            break;
    }
    return TEST_SUCCESS;
}

static int
libcache_test_deletion(void * libcache)
{
    cache_key_t key;
    libcache_scale_t max_entry_number = libcache_get_max_entry_number(libcache);
    key.imsi.val.imsi64bit = max_entry_number;
    do
    {
        liblb_cache_entry_t* entry;
        libcache_ret_t ret;
        entry = (liblb_cache_entry_t*)libcache_lookup(libcache, (void*)&key, NULL);
        if (entry == NULL)
        {
            if(key.imsi.val.imsi64bit == 0)
                break;
            printf("libcache_lookup after insertion, ERROR! key = %"PRIu64"\n",
                    key.imsi.val.imsi64bit);
            return TEST_FAILURE;
        }
        libcache_unlock_entry(libcache, entry);
        /*Validate user data that was set when the previous insertion.*/
        CHECK(entry->data.imsi.imsi64bit == key.imsi.val.imsi64bit);
        CHECK( entry->value == ((key.imsi.val.imsi64bit << 32) | (key.imsi.val.imsi64bit >> 32)));

        ret = libcache_delete_by_key(libcache, (void*)&key);

        CHECK(LIBCACHE_SUCCESS == ret);

        entry = (liblb_cache_entry_t*)libcache_lookup(libcache, (void*)&key, NULL);
        if (entry != NULL)
        {
            printf("hashtable_delete_by_node ERROR! key = %"PRIu64"\n",
                    key.imsi.val.imsi64bit);
            return TEST_FAILURE;
        }
		if(key.imsi.val.imsi64bit == 0)
		{
			break;
		}
        key.imsi.val.imsi64bit--;
    }
    while (1);
    return TEST_SUCCESS;
}

#define CHECK_TEST_RESULT(ret) {\
    if((ret) != TEST_SUCCESS) \
        return TEST_FAILURE;\
}

static int test_preperation_create_cache(void **libcache, libcache_scale_t scale)
{
    *libcache = libcache_create(
            scale,
            sizeof(liblb_cache_entry_t),
            sizeof(cache_key_t),
            malloc,
            free,
            NULL,
            cmp_key_imp,
            key_to_number_imp);
    return 0;
}

static int libcache_test_basic(void* libcache, int test_rounds)
{
    do
    {
        int test_ret;
        test_ret = libcache_test_insertion(libcache);
        CHECK_TEST_RESULT(test_ret);
        test_ret = libcache_test_deletion(libcache);
        CHECK_TEST_RESULT(test_ret);
    }
    while (test_rounds--);
    return TEST_SUCCESS;
}

static int libcache_test_destroy(void* libcache)
{
    if(libcache_destroy(libcache) == LIBCACHE_SUCCESS)
        return TEST_SUCCESS;
    return TEST_FAILURE;
}
#define MAX_CACHE_ENTRY 10240
#define TEST_ROUND 1000

TEST(libcache_unit_test)
{
    void * libcache;
    int retval;
    test_preperation_create_cache(&libcache, MAX_CACHE_ENTRY);
    GET_COST_USEC(retval = libcache_test_basic(libcache, TEST_ROUND));
    libcache_clean(libcache);
    CHECK(retval == TEST_SUCCESS);
    CHECK(libcache_get_entry_number(libcache) == 0);
    libcache_test_destroy(libcache);
 }
