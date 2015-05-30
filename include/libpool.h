#ifndef LIBPOOL_H_
#define LIBPOOL_H_
#include <stddef.h>
#include "libcache_def.h"

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
    void* start_memory;
    size_t memory_size;
    size_t element_size;
    node_t** element_link;
    pool_cb_t cb;
} element_pool_t;

typedef enum return_t {
    OK, ERR
} return_t;

/**
 * @fn pool_init
 *
 * @brief Init memory to pool..
 * @param [in] size - pool size
 * @return -  pool pointer, return NULL when failed
 */
element_pool_t* pool_init(size_t size,
                          LIBCACHE_ALLOCATE_MEMORY* allocate_memory,
                          LIBCACHE_FREE_MEMORY* free_memory);

/**
 * @fn pool_init_element_pool
 *
 * @brief Init element pool.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @param [in] entry_size  - entry size
 * @param [in] entry_count - entry count
 * @return -  OK / ERR
 */
int pool_init_element_pool(element_pool_t *pool, size_t entry_size, int entry_count);

/**
 * @fn pool_get_element
 *
 * @brief get an unused element memory.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @return -  a point to element memory (NULL for failed)
 */
void* pool_get_element(element_pool_t *pool);

/**
 * @fn pool_free_element
 *
 * @brief free an used element memory.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @param [in] element  - element address
 * @return -  OK / ERR
 */
int pool_free_element(element_pool_t *pool, void* element);

#ifdef __cplusplus
}
#endif
#endif /* LIBPOOL_H_ */
