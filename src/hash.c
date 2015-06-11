/*
 * hash.c
 *
 *  Created on: May 24, 2015
 *      Author: goyuan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hash.h"
#include "libpool.h"

#define HASH_BITS 16
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

void hash_free_node(node_t* node, void* pool_handle)
{
    if (node == NULL || pool_handle == NULL) {
        DEBUG_ERROR("input parameter %s is null.", "node");
        return;
    }
    hash_data_t* hd = (hash_data_t*) node->usr_data;
    if (hd != NULL) {
        if (hd->key != NULL) {
            pool_free_element(pool_handle, POOL_TYPE_KEY_SIZE, hd->key);
        }
        pool_free_element(pool_handle, POOL_TYPE_HASH_DATA_T, hd);
    }
    pool_free_element(pool_handle, POOL_TYPE_NODE_T, node);
    return;
}

void* hash_init(u32 max_entry,
                size_t key_size,
                LIBCACHE_CMP_KEY* key_cmp,
                LIBCACHE_KEY_TO_NUMBER* key_to_num,
                void *pool_handle)
{
    if (pool_handle == NULL) {
        DEBUG_ERROR("pool_handle can not be NULL.");
        return NULL;
    }

    hash_t* hash = (hash_t*) pool_get_element(pool_handle, POOL_TYPE_HASH_T);
    if (hash == NULL) {
        DEBUG_ERROR("%s init failed: ouf of memory!", "hash");
        return NULL;
    }

    hash->bucket_list = (bucket_t*) pool_get_element(pool_handle, POOL_TYPE_BUCKET_T);
    hash->entry_count = 0;
    hash->key_size = key_size;
    hash->kcmp = key_cmp;
    hash->k2num = key_to_num;
    hash->max_entry = 65536;

    int i = 0;
    while (i < max_entry) {
        hash->bucket_list[i].list_count = 0;
        hash->bucket_list[i].list = NULL;
        i++;
    }
    return hash;
}

void* hash_add(void* hash_table, const void* key, void* hash_node, void* cache_node, void* pool_handle)
{

    if (hash_table == NULL || key == NULL) {
        DEBUG_ERROR("input parameter %s %s is null.",
                (NULL == hash_table) ? "hash_table" : "",
                (NULL == key) ? "key" : "");
        return NULL;
    }
    hash_t* hash = (hash_t*) hash_table;
    u32 hash_code = key_to_hash(hash, key);
    if (hash_code >= hash->max_entry) {
        DEBUG_ERROR("hash key is invalid: %d", hash_code);
        return NULL;
    }

    node_t* node = (node_t*) hash_node;
    if (node == NULL) {
        node = (node_t*) pool_get_element(pool_handle, POOL_TYPE_NODE_T);
        node->usr_data = (hash_data_t*) pool_get_element(pool_handle, POOL_TYPE_HASH_DATA_T);
        ((hash_data_t*) node->usr_data)->key = pool_get_element(pool_handle, POOL_TYPE_KEY_SIZE);
    }

    memset(((hash_data_t*) node->usr_data)->key, 0, hash->key_size);
    memcpy(((hash_data_t*) node->usr_data)->key, key, hash->key_size);

    ((hash_data_t*) node->usr_data)->cache_node_ptr = cache_node;
    node->next_node = NULL;
    node->previous_node = NULL;
    bucket_t* bucket = &(hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        bucket->list = (list_t*) pool_get_element(pool_handle, POOL_TYPE_LIST_T);
        bucket->list_count = 0;
        list_init(bucket->list);
    }
    list_push_back(bucket->list, node);

    bucket->list_count++;
    hash->entry_count++;
    DEBUG_INFO("Add hash key successfully,hash_code:%d", hash_code);
    return node;
}

void* hash_del(void* hash_table, const void* key, void* hash_node, void* pool_handle)
{
    if (hash_table == NULL || hash_node == NULL || key == NULL) {
        DEBUG_ERROR("input parameter %s %s %s is null.",
                (NULL == hash_table) ? "hash_table" : "",
                (NULL == key) ? "key" : "",
                (NULL == hash_node) ? "hash_node" : "");
        return NULL;
    }

    hash_t* hash = (hash_t*) hash_table;
    u32 hash_code = key_to_hash(hash, key);
    if (hash_code >= hash->max_entry) {
        DEBUG_ERROR("hash key is invalid: %d", hash_code);
        return NULL;
    }

    bucket_t* bucket = &(hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        DEBUG_ERROR("%s is NULL.", "hash_list");
        return NULL;
    } else {
        node_t* node = (node_t*) hash_node;
        list_remove(bucket->list, node);
    }
    bucket->list_count--;
    hash->entry_count--;
    return hash_node;
}

void* hash_find(void* hash_table, const void* key)
{
    hash_t *hash = (hash_t*) hash_table;
    u32 hash_code = key_to_hash(hash, key);
    if (hash_code >= hash->max_entry) {
        DEBUG_ERROR("hash key is invalid: %d", hash_code);
        return NULL;
    }
    bucket_t* bucket = &(hash->bucket_list[hash_code]);
    node_t* node = NULL;
    if (bucket->list != NULL) {
        node = bucket->list->head_node;
        while (node != NULL) {
            if (!hash->kcmp(key, ((hash_data_t*) node->usr_data)->key)) {
                break;
            }
            node = node->next_node;
        }
    }
    return node;
}

int hash_get_count(const void* hash_table)
{
    if (hash_table == NULL) {
        DEBUG_ERROR("input parameter %s is null.", "hash_table");
        return -1;
    }
    const hash_t* hash = (const hash_t*) hash_table;
    return hash->entry_count;
}

static void hash_release(void* hash_table, int is_destroy, void* pool_handle)
{
    if (hash_table == NULL) {
        DEBUG_ERROR("input parameter %s is null.", "hash_table");
        return;
    }
    hash_t* hash = (hash_t*) hash_table;
    int i = 0;
    for (i = 0; i < hash->max_entry; i++) {
        bucket_t* bucket = &(hash->bucket_list[i]);
        node_t *bucket_node;
        if (bucket->list != NULL) {
            while (NULL != (bucket_node = list_pop_front(bucket->list))) {
                hash_free_node(bucket_node, pool_handle);
            }
            (void) pool_free_element(pool_handle, POOL_TYPE_LIST_T, bucket->list);
            bucket->list = NULL;
            bucket->list_count = 0;
        }
    }
    if (is_destroy) {
        pool_free_element(pool_handle, POOL_TYPE_BUCKET_T, hash->bucket_list);
        pool_free_element(pool_handle, POOL_TYPE_HASH_T, hash);
    } else {
        hash->entry_count = 0;
    }
}

void hash_free(void* hash, void* pool_handle)
{
    hash_release(hash, FALSE, pool_handle);
}

void hash_destroy(void* hash, void* pool_handle)
{
    hash_release(hash, TRUE, pool_handle);
}
