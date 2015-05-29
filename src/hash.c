/*
 * hash.c
 *
 *  Created on: 2015Äê5ÔÂ24ÈÕ
 *      Author: goyuan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

#define HASH_BIT 16

#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

static void* g_key = NULL;
key_compare g_kcmp = NULL;


static inline u32 hash_32(u32 val, u32 bits)
{
    u32 hash = val * GOLDEN_RATIO_PRIME_32;
    return hash >> (32 - bits);
}

static u32 key_to_hash(hash_t* hash, void* key)
{

    u32 value = hash->k2num(key);
    return hash_32(value, HASH_BIT);
}

static int find_node(node_t* node)
{
    if (node == NULL) {
        printf("fatal error, invalid parameter");
        return -1;
    }
    hash_data_t* hd = (hash_data_t*)node->usr_data;
    return g_kcmp(g_key, hd->key);
}

static void free_node(node_t* node)
{
    if (node == NULL) {
        return ;
    }
    hash_data_t* hd = (hash_data_t*) node->usr_data;
    if(hd != NULL)
    {
        if(hd->key != NULL)
        {
            free(hd->key);
        }
        free(hd);
    }
    free(node);
    return ;
}

void* hash_init(int key_size, key_compare key_cmp, key_to_number key_to_num)
{
    hash_t* hash = (hash_t*) malloc(sizeof(hash_t));
    if (hash == NULL) {
        printf("hash init failed: ouf of memory!");
        return NULL;
    }

    hash->entry_count = 0;
    hash->key_size = key_size;
    hash->kcmp = key_cmp;
    hash->k2num = key_to_num;

    int i = 0;
    while (i < MAX_BUCKETS) {
        hash->bucket_list[i].list_count = 0;
        hash->bucket_list[i].list = NULL;
        i++;
    }
    return hash;
}

void* hash_add(void* hash_table, void* key, void* cache_node)
{
    if (hash_table == NULL || key == NULL) {
        printf("invalid parameter\n");
        return NULL;
    }
    hash_t* hash = (hash_t*) hash_table;
    u32 hash_code = key_to_hash(hash, key);
    if (hash_code > MAX_BUCKETS) {
        printf("hash key is invalid:%d\n", hash_code);
        return NULL;
    }

    node_t* node = (node_t*) malloc(sizeof(node_t));
    if (node == NULL) {
        printf("get memory failed, out of memory!");
        return NULL;
    }
    node->usr_data = (hash_data_t*) malloc(sizeof(hash_data_t));
    hash_data_t* hd =(hash_data_t*)node->usr_data;
    hd->key = malloc(sizeof(hash->key_size) + 1);
    memset(hd->key, 0, hash->key_size + 1);
    memcpy(hd->key, key, hash->key_size);

    hd->cache_node_ptr = cache_node;
    node->next_node = NULL;
    node->previous_node = NULL;
    bucket_t* bucket = &(hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        bucket->list = (list_t*) malloc(sizeof(list_t));
        bucket->list_count = 0;
        list_init(bucket->list);
    }
    list_push_back(bucket->list, node);

    bucket->list_count++;
    hash->entry_count++;
    return node;
}

int hash_del(void* hash_table, void* key, void* hash_node)
{
    if (hash_table == NULL || hash_node == NULL || key == NULL) {
        printf("invalid parameter");
        return -1;
    }
    hash_t* hash = (hash_t*) hash_table;

    u32 hash_code = key_to_hash(hash, key);
    if (hash_code >= MAX_BUCKETS) {
        printf("hash key is invalid");
        return -1;
    }

    bucket_t* bucket = &(hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        printf("Fatal error, hash_list is NULL");
        return -1;
    } else {
        node_t* node = (node_t*) hash_node;
        list_remove(bucket->list, node);
        free_node(node);
    }
    bucket->list_count--;
    hash->entry_count--;
    return 0;
}

void* hash_find(void* hash_table, void* key)
{
    if (hash_table == NULL || key == NULL) {
        return NULL;
    }

    hash_t *hash = (hash_t*) hash_table;
    u32 hash_code = key_to_hash(hash, key);
    if (hash_code >= MAX_BUCKETS) {
        printf("hash key is invalid");
        return NULL;
    }
    bucket_t* bucket = &(hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        printf("Fatal error, hash_list is NULL");
        return NULL;
    } else {
        g_key = key;
        g_kcmp = hash->kcmp;
        node_t* node = list_foreach(bucket->list, find_node);
        if (node == NULL) {
            printf("Can't find the key\n");
            return NULL;
        }
        return node;
    }
}

int hash_get_count(void* hash_table)
{
    if (hash_table == NULL) {
        return -1;
    }
    hash_t* hash = (hash_t*) hash_table;
    return hash->entry_count;
}


void hash_free(void* hash_table)
{
    if(hash_table == NULL)
    {
        return;
    }
    hash_t* hash = (hash_t*)hash_table;
    int i = 0;
    for(i = 0; i<=MAX_BUCKETS; i++)
    {
        bucket_t* bucket = &(hash->bucket_list[i]);
        if(bucket->list != NULL)
        {
            list_clear(bucket->list, free_node);
            free(bucket->list);
        }
    }
    free(hash);
}
