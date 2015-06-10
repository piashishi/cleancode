#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libcache.h"
#include "libcache_def.h"
#include "libpool.h"
#include "hash.h"

typedef struct libcache_node_usr_data_t
{
    void* key;
    node_t* hash_node_ptr;
    void* pool_element_ptr;
    uint32_t lock_counter;
}libcache_node_usr_data_t;

typedef struct libcache_t
{
    void* pool;
    void* hash_table;
    list_t* list;
    size_t entry_size;
    size_t key_size;
    libcache_scale_t max_entry_number;
    LIBCACHE_FREE_MEMORY* free_memory;
    LIBCACHE_FREE_ENTRY* free_entry;
}libcache_t;

/*
 *  @brief libcache_create    creates a cache object
 *
 *  @param max_entry_number      maximum entry number that this cache is able to store.
 *  @param entry_size            size of an entry, bytes
 *  @param key_size              size of a key, bytes
 *  @param allocate_memory       function to allocate memory for this cache object, e.g. malloc().
 *  @param free_memory           function to free whole cache object, e.g. free().
 *  @param free_entry            function to free entry and key, it can be NULL if there isn't any resource to release.
 *  @param cmp_key               function to compare two keys, e.g. hash table, avl tree can use it.
 *  @param key_to_number         function to translate key to a number, e.g. hash table can use it.
 *  @return                      pointer of a cache object.
 */
void* libcache_create(
        libcache_scale_t max_entry_number,
        size_t entry_size,
        size_t key_size,
        LIBCACHE_ALLOCATE_MEMORY* allocate_memory,
        LIBCACHE_FREE_MEMORY* free_memory,
        LIBCACHE_FREE_ENTRY* free_entry,
        LIBCACHE_CMP_KEY* cmp_key,
        LIBCACHE_KEY_TO_NUMBER* key_to_number)
{
    if (allocate_memory == NULL || free_memory == NULL) {
        DEBUG_ERROR("argument %s and %s can not be NULL.", "allocate_memory", "free_memory");
        return NULL;
    }


    pool_attr_t pool_attr[] = {
            { entry_size, max_entry_number },
            { sizeof(libcache_t), 1 } ,
            { sizeof(list_t), 1 + max_entry_number},
            { sizeof(node_t), max_entry_number * 2 + 1},
            { sizeof(libcache_node_usr_data_t), max_entry_number },
            { key_size, max_entry_number * 2},
            { sizeof(hash_t), 1 }, // POOL_TYPE_HASH_T
            { sizeof(bucket_t)*(max_entry_number+1), 1 }, // POOL_TYPE_BUCKET_T
            { sizeof(hash_data_t), max_entry_number+1},
            };


    size_t large_mem_size = pool_caculate_total_length(POOL_TYPE_MAX, pool_attr);

    void *large_memory = allocate_memory(large_mem_size);
    if (large_memory == NULL) {
        DEBUG_ERROR("Memory malloc failed!")
    }

    void * pools = pools_init(large_memory, large_mem_size, POOL_TYPE_MAX, pool_attr);

    libcache_t* libcache = (libcache_t*) pool_get_element(pools, POOL_TYPE_LIBCACHE_T);
    libcache->pool = pools;

    libcache->hash_table = hash_init(max_entry_number, key_size, cmp_key, key_to_number, libcache->pool);

    libcache->list = (list_t*) pool_get_element(pools, POOL_TYPE_LIST_T);
    list_init(libcache->list);

    libcache->entry_size = entry_size;
    libcache->key_size = key_size;
    libcache->max_entry_number = max_entry_number;
    libcache->free_memory = free_memory;
    libcache->free_entry = free_entry;

    return libcache;
}

/*
 *  @brief libcache_lookup   To look up an cache entry with a given key.
 *
 *  @param libcache          cache object, cannot be NULL.
 *  @param key               key, cannot be NULL.
 *  @param dst_entry         a copy of entry that fetch by key. it could be NULL.
 *  @return NULL             didn't find out such entry with the key.
 *          pointer          points to an entry with the key.
 *  NOTE:  The entry in cache will be locked if dst_entry is NULL, one entry can be locked many times.
 *         libcache_unlock_entry should be called to unlock the entry when the entry is not being used this time.
 */
void* libcache_lookup(void* libcache, const void* key, void* dst_entry)
{
    libcache_t* libcache_ptr = (libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return NULL;
    }

    if (NULL == key) {
        DEBUG_ERROR("input parameter %s is null", "key");
        return NULL;
    }

    void* return_value = NULL;

    do {
        // Note: find the entry according to key
        node_t* hash_node = (node_t*)hash_find(libcache_ptr->hash_table, key);
        if (NULL == hash_node) {
            break;
        }

        node_t* libcache_node = (node_t*)((hash_data_t*)hash_node->usr_data)->cache_node_ptr;
        if (NULL == libcache_node) {
            break;
        }

        if (NULL == dst_entry) {
            // TODO: lock should be added here
            ((libcache_node_usr_data_t*)libcache_node->usr_data)->lock_counter++;

            return_value = ((libcache_node_usr_data_t*)libcache_node->usr_data)->pool_element_ptr;
        } else {
            // Note: copy into dst_entry and return NULL, no lock added too
            memcpy(dst_entry, ((libcache_node_usr_data_t*)libcache_node->usr_data)->pool_element_ptr, libcache_ptr->entry_size);
            return_value = dst_entry;
        }

        // Note: put the newest found node in front of list
       // list_remove(libcache_ptr->list, libcache_node);
       // list_push_front(libcache_ptr->list, libcache_node);

        list_swap_to_head(libcache_ptr->list, libcache_node);

    } while(0);

    return return_value;
}

/*
 *  @brief libcache_get_unlock_node  find unlocked node in list.
 *
 *  @param node             node in list.
 *  @return 0: traversing over; 1: continue traversing
 */
static inline int libcache_get_unlock_node(node_t *node) {
    return (((libcache_node_usr_data_t*)node->usr_data)->lock_counter) ? 1 : 0;
}

/*
 *  @brief libcache_add         attempts to add an entry with a given key.
 *
 *  @param libcache             cache object, cannot be NULL.
 *  @param key                  key, cannot be NULL.
 *  @param src_entry            entry with an expected value to add.
 *  @return NULL                could not add the entry because an entry with the same key is existing.
 *          pointer             points to an entry with the key, so user can write value to it.
 *  NOTE:   The entry in cache will be locked if src_entry is NULL, one entry can be locked many times.
 *          libcache_unlock_entry should be called to unlock the e ntry when the entry is not being used this time.
 */
void* libcache_add(void * libcache, const void* key, const void* src_entry)
{
libcache_t* libcache_ptr = (libcache_t*) libcache;
if (NULL == libcache_ptr) {
    DEBUG_ERROR("input parameter %s is null", "libcache");
    return NULL;
}

if (NULL == key) {
    DEBUG_ERROR("input parameter %s is null", "key");
    return NULL;
}

void* return_value = NULL;

// Note: find node, if node isn't existed and add it
do {
    // Note: find node from hash by key, so not add the data
    node_t* hash_node = (node_t*) hash_find(libcache_ptr->hash_table, key);
    if (NULL != hash_node) {
        DEBUG_INFO("the key is existed in cache");
        break;
    }

    node_t* unlock_node = NULL;
    libcache_node_usr_data_t* cache_data;

    // Note: if cache pool is full, check unlocked node in libcache list back
    if (libcache_ptr->max_entry_number <= libcache_ptr->list->total_nodes) {
        // Note: if no unlocked node in libcache list, return directly
        DEBUG_INFO("the cache is full, try to swap old data out");
        unlock_node = list_reverse_foreach(libcache_ptr->list, libcache_get_unlock_node);
        if (NULL == unlock_node) {
            DEBUG_INFO("all data are in use, swap failed!");
            break;
        } else { // Note: if have unlocked node in libcache list
            DEBUG_INFO("swap data successfully!");
            list_swap_to_head(libcache_ptr->list, unlock_node);
            cache_data = (libcache_node_usr_data_t*) unlock_node->usr_data;

            hash_del(libcache_ptr->hash_table, cache_data->key, cache_data->hash_node_ptr, libcache_ptr->pool);
            memset(cache_data->key, 0, libcache_ptr->key_size);
        }
    } else { // Note: if cache pool is not full, create new node
        unlock_node = (node_t*) pool_get_element(libcache_ptr->pool, POOL_TYPE_NODE_T);
        unlock_node->usr_data = pool_get_element(libcache_ptr->pool, POOL_TYPE_LIBCACHE_NODE_USR_DATA_T);
        cache_data = (libcache_node_usr_data_t*) unlock_node->usr_data;
        cache_data->key = pool_get_element(libcache_ptr->pool, POOL_TYPE_KEY_SIZE);
        cache_data->pool_element_ptr = pool_get_element(libcache_ptr->pool, POOL_TYPE_DATA);
        cache_data->lock_counter = 0;

        list_push_front(libcache_ptr->list, unlock_node);
        pool_set_reserved_pointer(cache_data->pool_element_ptr, (void*) unlock_node);
    }

    if (NULL != src_entry) {
        memcpy(cache_data->pool_element_ptr, src_entry, libcache_ptr->entry_size);
    } else {
        cache_data->lock_counter++;
    }
    memcpy(cache_data->key, key, libcache_ptr->key_size);
    // Note: add node into hash
    cache_data->hash_node_ptr = hash_add(libcache_ptr->hash_table, key, unlock_node, libcache_ptr->pool);
    return_value = cache_data->pool_element_ptr;
} while (0);

return return_value;
}

/*
 *  @brief libcache_delete_by_key attempts to delete an entry with a given key.
 *
 *  @param libcache               cache object, cannot be NULL.
 *  @param key                    key, cannot be NULL.
 *  @return
 *          LIBCACHE_NOT_FOUND    entry wasn't found.
 *          LIBCACHE_LOCKED       the entry was unable to deleted because it's locked.
 *          LIBCACHE_SUCCESS      the entry was deleted successfully.
 */
libcache_ret_t  libcache_delete_by_key(void * libcache, const void* key)
{
    libcache_t* libcache_ptr = (libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }

    if (NULL == key) {
        DEBUG_ERROR("input parameter %s is null", "key");
        return LIBCACHE_FAILURE;
    }

    libcache_ret_t return_value = LIBCACHE_SUCCESS;
    do {
        node_t* hash_node = (node_t*)hash_find(libcache_ptr->hash_table, key);
        if (NULL == hash_node) {
            return_value = LIBCACHE_NOT_FOUND;
            break;
        }

        // Note: if the entry is locked, just return
        node_t* libcache_node = (node_t*)((hash_data_t*)hash_node->usr_data)->cache_node_ptr;
        libcache_node_usr_data_t* libcache_node_usr_data = (libcache_node_usr_data_t*)libcache_node->usr_data;
         if (libcache_node_usr_data->lock_counter > 0) {
             return_value = LIBCACHE_LOCKED;
             break;
         }

        // Note: delete node from hash
        (void) hash_del(libcache_ptr->hash_table, key, libcache_node_usr_data->hash_node_ptr, libcache_ptr->pool);

        // Note: delete node from pool
        pool_free_element(libcache_ptr->pool, POOL_TYPE_DATA, libcache_node_usr_data->pool_element_ptr);

        // Note: delete node from list
        list_remove(libcache_ptr->list, libcache_node);

        // Note: free node resource
        pool_free_element(libcache_ptr->pool, POOL_TYPE_LIBCACHE_NODE_USR_DATA_T, libcache_node_usr_data);
        pool_free_element(libcache_ptr->pool, POOL_TYPE_KEY_SIZE, ((libcache_node_usr_data_t*)libcache_node->usr_data)->key);
        pool_free_element(libcache_ptr->pool, POOL_TYPE_NODE_T, libcache_node);

        return_value = LIBCACHE_SUCCESS;
    } while(0);

    return return_value;
}

/*
 *  @brief libcache_delete_entry   attempts to delete an entry.
 *
 *  @param libcache                cache object, cannot be NULL.
 *  @param entry                   entry (returned by libcache_lookup/libcache_add) in the cache.
 *  @return
*           LIBCACHE_NOT_FOUND     entry wasn't found.
*           LIBCACHE_LOCKED        entry was unable to deleted while it's locked.
 *          LIBCACHE_SUCCESS       entry was deleted successfully.
 */
libcache_ret_t  libcache_delete_entry(void * libcache, void* entry)
{
    libcache_t* libcache_ptr = (libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }

    libcache_ret_t return_value = LIBCACHE_FAILURE;

    do {
        // Note: judge whether entry is existed in cache
        node_t* libcache_node = pool_get_reserved_pointer(entry);
        if (libcache_node == NULL) {
            return_value = LIBCACHE_NOT_FOUND;
            break;
        }

        // Note: judge whether entry is locked
        libcache_node_usr_data_t* libcache_node_usr_data = (libcache_node_usr_data_t*)libcache_node->usr_data;
        if (libcache_node_usr_data->lock_counter > 0) {
            return_value = LIBCACHE_LOCKED;
            break;
        }

        hash_data_t* hash_data = (hash_data_t*) (libcache_node_usr_data->hash_node_ptr->usr_data);
        return_value = libcache_delete_by_key(libcache_ptr, hash_data->key);
    } while(0);

    return return_value;
}

/*
 *  @brief libcache_unlock_entry    attempts to unlock an entry.
 *
 *  @param libcache                 cache object, cannot be NULL.
 *  @param entry                    entry (returned by libcache_lookup/libcache_add) in the cache.
 *  @return
 *          LIBCACHE_UNLOCKED       the entry is already unlocked,
 *                                  this indicates unpaired locking & unlocking issue happened.
 *          LIBCACHE_SUCCESS        the entry was unlocked successfully once.
 */
libcache_ret_t libcache_unlock_entry(void * libcache, void* entry)
{
    libcache_t* libcache_ptr = (libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }

    if (entry == NULL) {
        DEBUG_ERROR("input parameter %s is null", "entry");
        return LIBCACHE_FAILURE;
    }

    libcache_ret_t return_value = LIBCACHE_FAILURE;

    node_t* libcache_node = pool_get_reserved_pointer(entry);

    if (NULL == libcache_node) {
        return_value = LIBCACHE_NOT_FOUND;
    } else {
        // Note: unlock entry
        libcache_node_usr_data_t* libcache_node_usr_data = (libcache_node_usr_data_t*)libcache_node->usr_data;
        if (libcache_node_usr_data->lock_counter == 0) {
            return_value = LIBCACHE_UNLOCKED;
        } else {
            libcache_node_usr_data->lock_counter--;
            return_value = LIBCACHE_SUCCESS;
        }
    }

    return return_value;
}

/*
 *  @brief libcache_get_max_entry_number    gets a capacity of the maximum number of entries this cache can store.
 *
 *  @param libcache                         cache object, cannot be NULL.
 *  @return
 *         the number
 */
libcache_scale_t libcache_get_max_entry_number(const void * libcache)
{
    const libcache_t* libcache_ptr = (const libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }
    return libcache_ptr->max_entry_number;
}

/*
 *  @brief libcache_get_entry_number         gets the number of entries this cache stores.
 *
 *  @param libcache                          cache object, cannot be NULL.
 *  @return
 *         the number
 */
libcache_scale_t libcache_get_entry_number(const void * libcache)
{
    const libcache_t* libcache_ptr = (const libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }

    return hash_get_count(libcache_ptr->hash_table);
}

/*
 *  @brief libcache_clean         attempts to delete all entries.
 *
 *  @param libcache               cache object, cannot be NULL.
 *  @return
 *      LIBCACHE_LOCKED           this operation aborted while some entries were locked.
 *      LIBCACHE_SUCCESS          all entries were deleted successfully,
 *                                now the cache is empty as fresh as just created.
 */
libcache_ret_t libcache_clean(void * libcache)
{
    libcache_t* libcache_ptr = (libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }

    node_t* libcache_node = NULL;
    while (NULL != (libcache_node = list_pop_front(libcache_ptr->list))) {
        libcache_node_usr_data_t* libcache_node_usr_data = (libcache_node_usr_data_t*)libcache_node->usr_data;
        pool_free_element(libcache_ptr->pool, POOL_TYPE_DATA, libcache_node_usr_data->pool_element_ptr);
        pool_free_element(libcache_ptr->pool, POOL_TYPE_LIBCACHE_NODE_USR_DATA_T, libcache_node_usr_data);
        pool_free_element(libcache_ptr->pool, POOL_TYPE_NODE_T, libcache_node);
    }

    hash_free(libcache_ptr->hash_table, libcache_ptr->pool);
    return LIBCACHE_SUCCESS;
}

/*
 *  @brief libcache_destroy         attempts to free all entries, then destroy this cache.
 *
 *  @param libcache                 cache object, cannot be NULL.
 *  @return
 *      LIBCACHE_LOCKED             this operation aborted while some entries were locked.
 *      LIBCACHE_SUCCESS            all entries were deleted successfully, then cache was also destroyed after that.
 */
libcache_ret_t libcache_destroy(void * libcache)
{
    libcache_t* libcache_ptr = (libcache_t*)libcache;
    if (NULL == libcache_ptr) {
        DEBUG_ERROR("input parameter %s is null", "libcache");
        return LIBCACHE_FAILURE;
    }

    node_t* libcache_node = NULL;
    while (NULL != (libcache_node = list_pop_front(libcache_ptr->list))) {
        libcache_node_usr_data_t* libcache_node_usr_data = (libcache_node_usr_data_t*)libcache_node->usr_data;
        if (libcache_ptr->free_entry != NULL) {
            hash_data_t* hash_data = (hash_data_t*) (libcache_node_usr_data->hash_node_ptr->usr_data);
            libcache_ptr->free_entry(hash_data->key, libcache_node_usr_data->pool_element_ptr);
        }
    }

    hash_destroy(libcache_ptr->hash_table, libcache_ptr->pool);
    libcache_ptr->free_memory(libcache_ptr->pool);

    return LIBCACHE_SUCCESS;
}
