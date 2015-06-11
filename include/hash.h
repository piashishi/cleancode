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

#define HASH_BITS 16
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
#define HASH_BUCKETS 65536 //2^15 + 1

typedef struct hash_data_t {
    void* key;
    char* cache_node_ptr;
}__attribute__((aligned(8))) hash_data_t;

typedef struct bucket_t {
    list_t* list;
    int list_count;
}__attribute__((aligned(8))) bucket_t;

typedef struct hash_t {
    bucket_t* bucket_list;
    LIBCACHE_CMP_KEY* kcmp;
    LIBCACHE_KEY_TO_NUMBER* k2num;
    int max_buckets;
    int entry_count;
    int key_size;
}__attribute__((aligned(8))) hash_t;

inline u32 key_to_hash(hash_t* hash, const void* key)
{
    u32 val = (hash->k2num(key)) * GOLDEN_RATIO_PRIME_32;
    return val >> (32 - HASH_BITS);
}

/**
 * @fn hash_init
 *
 * @brief create hash table and initialization
 * @param [in] key_size - key length
 * @param [in] key_cmp - callback for compare key value.
 * @param [in] key_to_num - callback for convert key to number
 * @param [in] pool_handle - memory pool address
 * @return NULL  - when out of memory.
 * @return pointer to hash table
 */
void* hash_init(size_t key_size, LIBCACHE_CMP_KEY* key_cmp, LIBCACHE_KEY_TO_NUMBER* key_to_num, void *pool_handle);

/**
 * @fn hash_add
 *
 * @brief add cache list node to hash table.Notice, the hash_n
 * @param [in] hash - hash table
 * @param [in] key
 * @param [in] hash_node - hash node for add. if NULL, hash will allocate internal.
 * @param [in] cache_node - cache list node
 * @param [in] pool_handle - memory pool address
 * @return NULL  - when out of memory.
 * @return pointer to hash list node
 */
void* hash_add(void* hash, const void* key, void* hash_node, void* cache_node, void* pool_handle);

/**
 * @fn hash_del
 *
 * @brief delete hash list node by key.Notice the delete node should free by
 * application.
 * @param [in] hash - hash table
 * @param [in] key
 * @param [in] hash_node - hash list node
 * @param [in] pool_handle - memory pool address
 * @return hash node  - the deleted hash code
 * @return NULL when delete fail
 */
void* hash_del(void* hash, const void* key, void* hash_node, void* pool_handle);

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
 * @param [in] pool_handle - memory pool address
 */
void hash_free(void* hash, void* pool_handle);

/**
 * @fn hash_destroy
 *
 * @brief destroy hash_table
 * @param [in] hash - hash table
 * @param [in] pool_handle - memory pool address
 */
void hash_destroy(void* hash, void* pool_handle);

/**
 * @fn hash_free_node
 *
 * @brief free hash node.
 * @param [in] hash - hash table
 * @param [in] pool_handle - memory pool address
 */
void hash_free_node(node_t* node, void* pool_handle);

#endif

