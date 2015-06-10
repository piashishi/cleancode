#ifndef LIBPOOL_H_
#define LIBPOOL_H_
#include <stddef.h>
#include "libcache_def.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pool_cb_t {
    LIBCACHE_ALLOCATE_MEMORY* allocate_memory;
    LIBCACHE_FREE_MEMORY* free_memory;
} pool_cb_t;

typedef struct element_pool_t {
    list_t busy_list;
    list_t free_list;
    size_t element_size;
    int element_acount;
}__attribute__((packed))  element_pool_t;

typedef struct element_usr_data_t{
    int check_value;
    void* reserved_pointer;
    node_t* to_node;
} __attribute__((packed)) element_usr_data_t;

typedef struct pool_attr_t {
    size_t entry_size;
    libcache_scale_t entry_acount;
} pool_attr_t;

typedef enum {
    POOL_TYPE_DATA,
    POOL_TYPE_LIBCACHE_T,
    POOL_TYPE_LIST_T,
    POOL_TYPE_NODE_T,
    POOL_TYPE_LIBCACHE_NODE_USR_DATA_T,
    POOL_TYPE_KEY_SIZE,
    POOL_TYPE_HASH_T,
    POOL_TYPE_BUCKET_T,
    POOL_TYPE_HASH_DATA_T,
    POOL_TYPE_MAX,
} pool_type_e;

size_t pool_caculate_total_length(int pool_acount, pool_attr_t pool_attr[]);

void* pools_init(void* large_memory, size_t large_mem_size, int pool_acount, pool_attr_t pool_attr[]);

/**
 * @fn pool_init
 *
 * @brief Init memory to pool..
 * @param [in] size - pool size
 * @param allocate_memory       function to allocate memory for this cache object, e.g. malloc().
 * @param free_memory           function to free whole cache object, e.g. free().
 * @return -  pool pointer, return NULL when failed
 */

/**
 * @fn pool_init_element_pool
 *
 * @brief Init element pool.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @param [in] entry_size  - entry size
 * @param [in] entry_count - entry count
 * @return -  OK / ERR
 */

/**
 * @fn pool_get_element
 *
 * @brief get an unused element memory.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @return -  a point to element memory (NULL for failed)
 */
void* pool_get_element(void* pools, int pool_type);

/**
 * @fn pool_free_element
 *
 * @brief free an used element memory.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @param [in] element  - element address
 * @return -  OK / ERR
 */
return_t pool_free_element(void* pools, int pool_type, void* element);

/**
 * @fn pool_get_key_by_element_address
 *
 * @brief get key by element memory.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @param [in] element  - element address
 * @return - a point to element memory(NULL: not found or parameter error)
 */
return_t pool_set_reserved_pointer(void* element, void* to_set);
void* pool_get_reserved_pointer(void* element);

#ifdef __cplusplus
}
#endif
#endif /* LIBPOOL_H_ */
