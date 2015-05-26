/*
 * hash.h
 *
 *  Created on: 2015Äê5ÔÂ24ÈÕ
 *      Author: goyuan
 */

#ifndef HASH_H_
#define HASH_H_

#include "list.h"
#define MAX_BUCKETS 65535

#define u32  unsigned int



typedef int (*key_compare)(const void* src_key, const void* dst_key);
typedef u32 (*key_to_number)(const void* key);


typedef struct bucket_t
{
    list_t* list;
    int list_count;
}bucket_t;

typedef struct hash_t
{
    int entry_count;
    bucket_t bucket_list[MAX_BUCKETS];
}hash_t;

int hash_init(int max_entry, int key_size, key_compare key_cmp, key_to_number key_to_num);

void* hash_add(void* key, void* value);

int hash_del(void* key, void* entry);

void* hash_find(void* key);

int hash_get_count();

#endif

