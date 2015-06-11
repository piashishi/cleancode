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
    list_t free_list;
    long long  element_size;
    long long element_acount;
} __attribute__((packed)) element_pool_t;

typedef struct element_usr_data_t{
    node_t* to_node;
    void* reserved_pointer;
    int check_value;
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

/**
 * @fn pools_init
 *
 * @brief Init memory to pool..
 * @param [in] large_memory   - a memory pointer
 * @param [in] large_mem_size - size of the memory
 * @param [in] pool_count     - count of pools
 * @param [in] pool_attr      - describe the entry size and entry count of pool
 * @return -  pools pointer, return NULL when failed
 */
void* pools_init(void* large_memory, size_t large_mem_size, int pool_count, pool_attr_t pool_attr[]);


/**
 * @fn pool_get_element
 *
 * @brief get an unused element memory.
 * @param [in] pools     - pools handle
 * @param [in] pool_type - the type of pool
 * @return -  a point to element memory (NULL for failed)
 */
void* pool_get_element(void* pools, int pool_type);

/**
 * @fn pool_free_element
 *
 * @brief free an used element memory.
 * @param [in] pools     - pools handle
 * @param [in] pool_type - the type of pool
 * @param [in] element   - the element to free
 */
void pool_free_element(void* pools, int pool_type, void* element);

/**
 * @fn pool_set_reserved_pointer
 *
 * @brief set a pointer value to the reservation of element
 * @param [in] element   - the element address
 * @param [in] to_set    - the pointer to set
 * @return -  OK / ERR
 */

return_t pool_set_reserved_pointer(void* element, void* to_set);
/**
 * @fn pool_get_reserved_pointer
 *
 * @brief get a pointer value to the reservation of element
 * @param [in] element   - the element address
 * @return - the pointer
 */
void* pool_get_reserved_pointer(void* element);

#ifdef __cplusplus
}
#endif
#endif /* LIBPOOL_H_ */
