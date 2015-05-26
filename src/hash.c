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

hash_t* g_hash = NULL;
static int g_key_size = 0;
static void* g_key = NULL;
static int g_max_entry = 0;

#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

static key_compare compare_cb;
static key_to_number key_to_num_cb;

static inline u32 hash_32(u32 val, u32 bits)
{
    u32 hash = val * GOLDEN_RATIO_PRIME_32;
    return hash >> (32 - bits);
}

static u32 key_to_hash(void* key)
{
    u32 value = key_to_num_cb(key);
    return hash_32(value, HASH_BIT);
}

int find_node(node_t* node)
{
    return compare_cb(g_key, node->key);
}

int hash_init(int max_entry, int key_size, key_compare key_cmp, key_to_number key_to_num)
{
    g_hash = (hash_t*) malloc(sizeof(hash_t));
    if (g_hash == NULL) {
        printf("hash init failed: ouf of memory!");
        return -1;
    }

    g_hash->entry_count = 0;
    g_key_size = key_size;
    compare_cb = key_cmp;
    key_to_num_cb = key_to_num;
    g_max_entry = max_entry;

    int i = 0;
    while (i < MAX_BUCKETS) {
        g_hash->bucket_list[i].list_count = 0;
        g_hash->bucket_list[i].list = NULL;
        i++;
    }
    return 0;
}

void*  hash_add(void* key, void* entry)
{
    if (g_hash->entry_count > g_max_entry) {
        printf("hash table is full, add failed!");
        return NULL;
    }

    u32 hash_code = key_to_hash(key);
    if (hash_code > MAX_BUCKETS) {
        printf("hash key is invalid:%d\n", hash_code);
        return NULL;
    }

    node_t* node = (node_t*) malloc(sizeof(node_t));
    if (node == NULL) {
        printf("get memory failed, out of memory!");
        return NULL;
    }
    node->key = malloc(g_key_size + 1);
    memset(node->key, 0, g_key_size + 1);
    memcpy(node->key, key, g_key_size);

    node->entry = entry;
    node->next_node = NULL;
    node->previous_node = NULL;
    bucket_t* bucket = &(g_hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        bucket->list = (list_t*) malloc(sizeof(list_t));
        bucket->list_count = 0;
        list_init(bucket->list);
    } 
    list_push_back(bucket->list, node);
    
    bucket->list_count++;
    g_hash->entry_count++;
    return node;
}

int hash_del(void* key, void* entry)
{
    if (entry == NULL || key == NULL) {
        return -1;
    }

    u32 hash_code = key_to_hash(key);
    if (hash_code >= MAX_BUCKETS) {
        printf("hash key is invalid");
        return -1;
    }

    bucket_t* bucket = &(g_hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        printf("Fatal error, hash_list is NULL");
        return -1;
    } else {
        node_t* node = (node_t*) entry;
        list_remove(bucket->list, node);
        if (node->key != NULL) {
            free(node->key);
        }
        free(node);
    }
    bucket->list_count--;
    g_hash->entry_count--;
    return 0;
}

void* hash_find(void* key)
{
    if (key == NULL) {
        return NULL;
    }

	g_key = key;
    u32 hash_code = key_to_hash(key);
    if (hash_code >= MAX_BUCKETS) {
        printf("hash key is invalid");
        return NULL;
    }
    bucket_t* bucket = &(g_hash->bucket_list[hash_code]);
    if (bucket->list == NULL) {
        printf("Fatal error, hash_list is NULL");
        return NULL;
    } else {
        node_t* node = list_foreach(bucket->list, find_node);
        if (node == NULL) {
            printf("Can't find the key\n");
            return NULL;
        }
        return node;
    }
}

void print_hash()
{
    printf("Total value:%d\n", g_hash->entry_count);
    int i = 0;
	u32 sum = 0;
    for(i=0; i<=MAX_BUCKETS; i++)
    {
        bucket_t bucket = g_hash->bucket_list[i];
        if(bucket.list != NULL)
        {
			sum += bucket.list_count;
        }
    }
	printf("total sum:%d\n", sum);

}

int hash_get_count()
{
	return g_hash->entry_count;
}

