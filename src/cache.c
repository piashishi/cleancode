#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcache.h"
#include "libcache_def.h"

#define GET_CACHE_ENTRT_ADDR(libcache, index)  \
                                            ((cache_entry_t*)((char*)(libcache) + sizeof(libcache_t) + \
                                            ((libcache_t*)((libcache))->max_entry)*sizeof(cache_entry_t*))\
                                            + (index))
#define GET_CACHE_CONFLICT_LIST_HEAD(libcache, index) \
       ((char*)(libcache) + sizeof(libcache_t)+ (index)*sizeof(cache_entry_t*))

#define GET_CACHE_ENTRY_KEY_ADDR(entry)  ((char*)(entry) + sizeof(cache_entry_t))
#define GET_CACHE_ENTRY_DATA_ADDR(entry, keysize)  ((char*)(entry) + sizeof(cache_entry_t) +(keysize))

typedef struct cache_entry_t
{
    struct cache_entry_t* unlock_next_ptr;
    struct cache_entry_t* unlock_prev_ptr;
    struct cache_entry_t* conflict_entry_next_ptr;
    struct cache_entry_t* conflict_entry_prev_ptr;
    struct cache_entry_t* free_list_next_ptr;
    int ref_counter;
    int hash_index;
}cache_entry_t;


typedef struct libcache
{
    cache_entry_t* unlock_list_header;
    cache_entry_t* unlock_list_tail;
    cache_entry_t* free_list_header;
    libcache_scale_t entry_count;
    libcache_scale_t max_entry;
    libcache_scale_t cache_entry_size;
    libcache_scale_t cache_key_size;
    libcache_scale_t entry_size;
    libcache_scale_t key_size;
    LIBCACHE_CMP_KEY* key_cmp;
    LIBCACHE_KEY_TO_NUMBER* key_to_num;
    LIBCACHE_FREE_ENTRY* entry_free;
    LIBCACHE_FREE_MEMORY* free_memory;
}libcache_t;


static cache_entry_t* libcache_find_conflict_entry(libcache_t* libcache, cache_entry_t* head, void* key)
{
    entry = head->conflict_entry_next_ptr;
    while (entry != NULL) {
        void* entry_key = (void*)GET_CACHE_ENTRY_KEY_ADDR(entry);
        if (!libcache->key_cmp(entry_key, key)) {
            break;
        }
        entry = entry->conflict_entry_next_ptr;
    }
    return entry;
}


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
    libcache_scale_t cache_entry_size = (entry_size>>3)<<3 + 8;
    libcache_scale_t cache_key_size = (key_size>>3)<<3 + 8;

    libcache_scale_t total_cache_size = sizeof(libcache_scale_t) + max_entry_number * sizeof(cache_entry_t*)
            + max_entry_number * (sizeof(cache_entry_t) + cache_entry_size + cache_key_size);

    libcache_t* cache_ptr = (libcache_t*) allocate_memory(total_cache_size);
    if (cache_ptr == NULL) {
        DEBUG_ERROR("libcache allocate fail!");
        return NULL;
    }
    memset(cache_ptr, 0, total_cache_size);
    cache_ptr->entry_free = free_entry;
    cache_ptr->free_memory = free_memory;
    cache_ptr->max_entry = max_entry_number;
    cache_ptr->key_cmp = cmp_key;
    cache_ptr->key_to_num = key_to_number;
    cache_ptr->cache_entry_size = cache_entry_size;
    cache_ptr->cache_key_size = cache_key_size;
    cache_ptr->entry_size = entry_size;
    cache_ptr->key_size = key_size;

    index = 0;

    cache_entry_t* entry  = NULL;
    for (index = 0; index < max_entry_number; index++) {
        entry= (cache_entry_t*) GET_CACHE_ENTRT_ADDR(cache_ptr, index);
        libcache_push_free_list_header(cache_ptr, entry);
    }
    return cache_ptr;
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
    int index = libcache_ptr->key_to_num(key);
    if(index > libcache_ptr->max_entry)
    {
        DEBUG_ERROR("libcache lookup failed, invalid key");
        return NULL;
    }
    cache_entry_t* conflict_head = (cache_entry_t*) GET_CACHE_CONFLICT_LIST_HEAD(libcache_ptr, index);
    cache_entry_t* entry = libcache_find_conflict_entry(libcache_ptr, conflict_head, key);
    if (entry == NULL) {
        DEBUG_ERROR("libcache lookup fail, not found");
        return NULL;
    }
    if(dst_entry == NULL)
    {
        entry->ref_counter++;
        return (void*)GET_CACHE_ENTRY_DATA_ADDR(entry, libcache_ptr->cache_key_size);
    }
    else {
        memcpy(dst_entry,
               (void*) GET_CACHE_ENTRY_DATA_ADDR(entry, libcache_ptr->cache_key_size),
               libcache_ptr->entry_size);
        return dst_entry;
    }
}

static void libcache_push_unlocked_list_head(libcache_t* libcache, cache_entry_t* entry)
{
    if(libcache->unlock_list_header == NULL) {
        //first insert
        entry->unlock_next_ptr = NULL;
        entry->unlock_prev_ptr = NULL;
        libcache->unlock_list_header = entry;
        libcache->unlock_list_tail = entry;
    }
    else
    {
        libcache->unlock_list_header->unlock_prev_ptr = entry;
        entry->unlock_next_ptr = libcache->unlock_list_header;
        entry->unlock_prev_ptr = NULL;
        libcache->unlock_list_header = entry;
    }
}

static cache_entry_t* libcache_pop_unlocked_list_tail(libcache_t* libcache)
{
    cache_entry_t* entry = libcache->unlock_list_tail;
    if (libcache->unlock_list_header == libcache->unlock_list_tail) {
        //only one
        libcache->unlock_list_header = NULL;
        libcache->unlock_list_tail = NULL;
    } else {
        entry->unlock_prev_ptr->unlock_next_ptr = NULL;
        libcache->unlock_list_tail = entry->unlock_prev_ptr;
    }
    return entry;
}

static void libcache_pop_unlocked_list_node(libcache_t* libcache, cache_entry_t* entry)
{
    cache_entry_t* tmp = NULL;
    if (libcache->unlock_list_header == entry) {
        //header
        tmp = entry->unlock_next_ptr;
        tmp->unlock_prev_ptr = NULL;
        libcache->unlock_list_header = tmp;
    } else if (libcache->unlock_list_tail == entry) {
        tmp = entry->unlock_prev_ptr;
        tmp->unlock_next_ptr = NULL;
        libcache->unlock_list_tail = tmp;
    } else {
        entry->unlock_next_ptr->conflict_entry_prev_ptr = entry->unlock_prev_ptr;
        entry->unlock_prev_ptr->unlock_next_ptr = entry->unlock_next_ptr;
    }
}

static void libcache_push_free_list_header(libcache_t* libcache, cache_entry_t* entry)
{
    memset(entry, 0, sizeof(cache_entry_t));
    if (libcache->free_list_header == NULL) {
        libcache->free_list_header = entry;
    } else {
        entry->free_list_next_ptr = libcache->free_list_header;
        libcache->free_list_header = entry;
    }
}

static cache_entry_t* libcache_pop_free_list_header(libcache_t* libcache)
{
    cache_entry_t* entry = libcache->free_list_header;
    if (entry->free_list_next_ptr == NULL) {
        //last element
        libcache->free_list_header = NULL;
    } else {
        libcache->free_list_header = entry->free_list_next_ptr;
    }
    memset(entry, 0, sizeof(cache_entry_t) + libcache->cache_key_size + libcache->cache_entry_size);
    return entry;
}

static void libcache_pop_conflict_list_node(cache_entry_t* head, cache_entry_t* entry)
{
    entry = head->conflict_entry_next_ptr;
    if(likyly(entry->conflict_entry_prev_ptr == NULL))
    {
        //head
        head->conflict_entry_next_ptr = NULL;
    }
    else if(entry->conflict_entry_next_ptr == NULL)
    {
        //tail
        entry->conflict_entry_prev_ptr->conflict_entry_next_ptr = NULL;
    }
    else
    {
        entry->conflict_entry_prev_ptr->conflict_entry_next_ptr = entry->conflict_entry_next_ptr;
        entry->conflict_entry_next_ptr->conflict_entry_prev_ptr = entry->conflict_entry_prev_ptr;
    }
}

static void libcache_push_conflict_list_head(cache_entry_t* head, cache_entry_t* entry)
{
    cache_entry_t* tmp = head->conflict_entry_next_ptr;
    if(head == NULL)
    {
        entry->conflict_entry_next_ptr = NULL;
        entry->conflict_entry_prev_ptr = NULL;
        head->conflict_entry_next_ptr = entry;
    }
    else
    {
        entry->conflict_entry_next_ptr = tmp;
        tmp->conflict_entry_prev_ptr = entry;
        head->conflict_entry_next_ptr = entry;
    }
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

    int index = libcache_ptr->key_to_num(key);
    if (index > libcache_ptr->max_entry) {
        DEBUG_ERROR("libcache add key fail, invalid key");
        return NULL;
    }
    cache_entry_t* conflict_head = (cache_entry_t*) GET_CACHE_CONFLICT_LIST_HEAD(libcache_ptr, index);
    cache_entry_t* entry = libcache_find_conflict_entry(libcache_ptr, conflict_head, key);
    if (entry != NULL) {
        DEBUG_ERROR("libcache add key fail, key already exist");
        return NULL;
    }
    if (unlikely(libcache_ptr->entry_count > libcache_ptr->max_entry)) {
        DEBUG_INFO("libcache is full, swap old data");
        entry = libcache_pop_unlocked_list_tail(libcache_ptr);
        libcache_pop_conflict_list_node(conflict_head, entry);
        memset(entry, 0, sizeof(cache_entry_t) + libcache->cache_key_size + libcache->cache_entry_size);
    } else {
        entry = libcache_pop_free_list_header(libcache_ptr);
    }

    entry->hash_index = index;
    memcpy((void*) GET_CACHE_ENTRY_KEY_ADDR(entry), key, libcache_ptr->key_size);
    if (src_entry == NULL) {
        entry->ref_counter++;
    } else {
        memcpy((void*) GET_CACHE_ENTRY_DATA_ADDR(entry, libcache_ptr->cache_key_size),
               src_entry,
               libcache_ptr->entry_size);
        libcache_push_unlocked_list_head(libcache, entry);
    }
    libcache_push_conflict_list_head(conflict_head, entry);
    libcache_ptr->entry_count++;

    return (void*) GET_CACHE_ENTRY_DATA_ADDR(entry, libcache_ptr->cache_key_size);
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
    libcache_t* libcache_ptr = (libcache_t*) libcache;

    int index = libcache_ptr->key_to_num(key);
    if (index > libcache_ptr->max_entry) {
        DEBUG_ERROR("libcache delete key fail, invalid key");
        return NULL;
    }
    cache_entry_t* conflict_head = (cache_entry_t*) GET_CACHE_CONFLICT_LIST_HEAD(libcache_ptr, index);
    cache_entry_t* entry = libcache_find_conflict_entry(libcache_ptr, conflict_head, key);
    if (entry == NULL) {
        DEBUG_ERROR("libcache delete fail, not found");
        return LIBCACHE_NOT_FOUND;
    }
    if(entry->ref_counter >0) {
        return LIBCACHE_LOCKED;
    }
    else {
        libcache_pop_conflict_list_node(conflict_head, entry);
        libcache_push_free_list_header(libcache_ptr, entry);
        libcache_pop_unlocked_list_node(libcache_ptr, entry);
        libcache_ptr->entry_count--;
        return LIBCACHE_SUCCESS;
    }
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
libcache_ret_t libcache_delete_entry(void * libcache, void* entry)
{
    libcache_t* libcache_ptr = (libcache_t*) libcache;

    cache_entry_t* entry_ptr = (cache_entry_t*) ((char*) entry - sizeof(cache_entry_t));
    cache_entry_t* conflict_head = (cache_entry_t*) GET_CACHE_CONFLICT_LIST_HEAD(libcache_ptr, entry_ptr->hash_index);
    if (entry->ref_counter > 0) {
        return LIBCACHE_LOCKED;
    } else {
        libcache_pop_conflict_list_node(conflict_head, entry);
        libcache_push_free_list_header(libcache_ptr, entry);
        libcache_pop_unlocked_list_node(libcache_ptr, entry);
        libcache_ptr->entry_count--;
        return LIBCACHE_SUCCESS;
    }
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
    libcache_t* libcache_ptr = (libcache_t*) libcache;

    cache_entry_t* entry_ptr = (cache_entry_t*) ((char*) entry - sizeof(cache_entry_t));
    entry->ref_counter--;
    if (entry->ref_counter == 0) {
        libcache_push_unlocked_list_head(libcache_ptr, entry_ptr);
        return LIBCACHE_UNLOCKED;
    } else if (entry->ref_counter > 0) {
        return LIBCACHE_SUCCESS;
    } else {
        return LIBCACHE_FAILURE;
    }
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
    return libcache_ptr->max_entry;
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
    return libcache_ptr->entry_count;
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
    libcache_ptr->unlock_list_header = NULL;
    libcache_ptr->unlock_list_tail = NULL;
    libcache_ptr->free_list_header = NULL;
    libcache_ptr->entry_count = 0;

    //clean conflict header
    memset(((char*)libcache)+sizeof(libcache_t), 0, libcache_ptr->max_entry*sizeof(cache_entry_t*));
    int i = 0;
    cache_entry_t* entry;
    for(i = 0; i<libcache_ptr->max_entry; i++)
    {
        entry = (cache_entry_t*)GET_CACHE_ENTRT_ADDR(libcache_ptr, i);
        void* key = (void*)GET_CACHE_ENTRY_KEY_ADDR(entry);
        void* data = (void*)GET_CACHE_ENTRY_DATA_ADDR(entry, libcache_ptr->cache_key_size);
        if(libcache_ptr->entry_free)
        {
            libcache_ptr->entry_free(key, data);
        }
        memset(entry, 0, sizeof(cache_entry_t));
        memset(key, 0, libcache_ptr->cache_key_size);
        memset(data, 0, libcache_ptr->cache_entry_size);
        libcache_push_free_list_header(libcache_ptr, entry);
    }
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
    libcache_clean(libcache);
    libcache_ptr->free_memory(libcache_ptr->pool);

    return LIBCACHE_SUCCESS;
}

