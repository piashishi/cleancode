/*
 * hash.h
 *
 *  Created on: May 24, 2015
 *      Author: goyuan
 */

#ifndef HASH_H_
#define HASH_H_

#include "list.h"
#include "libcache_def.h"

#define u32  unsigned int

typedef struct hash_data_t {
    void* key;
    char* cache_node_ptr;
} hash_data_t;

typedef struct bucket_t {
    list_t* list;
    int list_count;
} bucket_t;

typedef struct hash_t {
    int max_entry;
    int entry_count;
    int key_size;
    LIBCACHE_CMP_KEY* kcmp;
    LIBCACHE_KEY_TO_NUMBER* k2num;
    bucket_t* bucket_list;
} hash_t;

/**
 * @fn hash_init
 *
 * @brief create hash table and initialization
 * @param [in] max_entry - max entry size
 * @param [in] key_size - key length
 * @param [in] key_cmp - callback for compare key value.
 * @param [in] key_to_num - callback for convert key to number
 * @return NULL  - when out of memory.
 * @return pointer to hash table
 */
void* hash_init(u32 max_entry, size_t key_size, LIBCACHE_CMP_KEY* key_cmp, LIBCACHE_KEY_TO_NUMBER* key_to_num, void *pool_handle);

/**
 * @fn hash_add
 *
 * @brief add cache list node to hash table
 * @param [in] hash - hash table
 * @param [in] key
 * @param [in] cache_node - cache list node
 * @return NULL  - when out of memory.
 * @return pointer to hash list node
 */
void* hash_add(void* hash, const void* key, void* cache_node, void* pool_handle);

/**
 * @fn hash_del
 *
 * @brief delete hash list node by key
 * @param [in] hash - hash table
 * @param [in] key
 * @param [in] hash_node - hash list node
 * @return 0  - when delete successfully
 * @return -1 when delete fail
 */
int hash_del(void* hash, const void* key, void* hash_node, void* pool_handle);

/**
 * @fn hash_find
 *
 * @brief find cache list node by key
 * @param [in] hash - hash table
 * @param [in] key
 * @return NULL  - not found
 * @return pointer to hash list node
 */
void* hash_find(void* hash, const void* key);

/**
 * @fn hash_get_count
 *
 * @brief get current hash entry count
 * @param [in] hash - hash table
 * @return -1 when parameter is NULL
 * @return hash entry count
 */
int hash_get_count(const void* hash);

/**
 * @fn hash_free
 *
 * @brief free hash_table
 * @param [in] hash - hash table
 */
void hash_free(void* hash, void* pool_handle);

/**
 * @fn hash_destroy
 *
 * @brief destroy hash_table
 * @param [in] hash - hash table
 */
void hash_destroy(void* hash, void* pool_handle);

#endif

