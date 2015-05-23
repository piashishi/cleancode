/*
 * libcache.h
 *
 *  Created on: Mar 24, 2015
 *      Author: root
 */

#ifndef LIBCACHE_H_
#define LIBCACHE_H_
#include "libcache_def.h"

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
        LIBCACHE_KEY_TO_NUMBER* key_to_number);

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
void* libcache_lookup(void* libcache, const void* key, void* dst_entry);

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
void* libcache_add(void * libcache, const void* key, const void* src_entry);

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
libcache_ret_t  libcache_delete_by_key(void * libcache, const void* key);

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
libcache_ret_t  libcache_delete_entry(void * libcache, void* entry);

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
libcache_ret_t libcache_unlock_entry(void * libcache, void* entry);

/*
 *  @brief libcache_get_max_entry_number    gets a capacity of the maximum number of entries this cache can store.
 *
 *  @param libcache                         cache object, cannot be NULL.
 *  @return
 *         the number
 */
libcache_scale_t libcache_get_max_entry_number(const void * libcache);

/*
 *  @brief libcache_get_entry_number         gets the number of entries this cache stores.
 *
 *  @param libcache                          cache object, cannot be NULL.
 *  @return
 *         the number
 */
libcache_scale_t libcache_get_entry_number(const void * libcache);

/*
 *  @brief libcache_clean         attempts to delete all entries.
 *
 *  @param libcache               cache object, cannot be NULL.
 *  @return
 *      LIBCACHE_LOCKED           this operation aborted while some entries were locked.
 *      LIBCACHE_SUCCESS          all entries were deleted successfully,
 *                                now the cache is empty as fresh as just created.
 */
libcache_ret_t libcache_clean(void * libcache);

/*
 *  @brief libcache_destroy         attempts to free all entries, then destroy this cache.
 *
 *  @param libcache                 cache object, cannot be NULL.
 *  @return
 *      LIBCACHE_LOCKED             this operation aborted while some entries were locked.
 *      LIBCACHE_SUCCESS            all entries were deleted successfully, then cache was also destroyed after that.
 */
libcache_ret_t libcache_destroy(void * libcache);
#endif /* LIBCACHE_H_ */
