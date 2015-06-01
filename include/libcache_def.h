/*
 * libcache_def.h
 *
 *  Created on: Mar 25, 2015
 *      Author: root
 */

#ifndef LIBCACHE_DEF_H_
#define LIBCACHE_DEF_H_
#include <stddef.h>
#include <stdint.h>


/* It could be uint16_t or uint8_t to achieve
 * a smaller space usage when very few entries
 * are required.
 */
typedef  uint32_t libcache_scale_t;

#define TRUE 1
#define FALSE 0 

typedef enum
{
    LIBCACHE_EQU = 0,
    LIBCACHE_NOT_EQU,
    LIBCACHE_BIGER,
    LIBCACHE_SMALLER,
} libcache_cmp_ret_t;

typedef enum
{
    LIBCACHE_SUCCESS = 0,
    LIBCACHE_NOT_FOUND,
    LIBCACHE_LOCKED,
    LIBCACHE_UNLOCKED,
    LIBCACHE_FULL,
    LIBCACHE_FAILURE,
} libcache_ret_t;

typedef libcache_cmp_ret_t LIBCACHE_CMP_KEY(const void *key1, const void *key2);
typedef void* LIBCACHE_ALLOCATE_MEMORY(size_t size);
typedef void LIBCACHE_FREE_MEMORY(void* addr);
typedef void LIBCACHE_FREE_ENTRY(void* key, void* entry);
typedef libcache_scale_t LIBCACHE_KEY_TO_NUMBER(const void* key);
#endif /* LIBCACHE_DEF_H_ */
