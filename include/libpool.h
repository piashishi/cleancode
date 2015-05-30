#ifndef LIBPOOL_H_
#define LIBPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct element_pool_t {
    list_t busy_list;
    list_t free_list;
    void* start_memory;
    int memory_size;
    int element_size;
    node_t** element_link;
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
element_pool_t* pool_init(int size);

/**
 * @fn pool_init_element_pool
 *
 * @brief Init element pool.
 * @param [in] pool - pool pointer, should initiated by pool_init first.
 * @param [in] entry_size  - entry size
 * @param [in] entry_count - entry count
 * @return -  OK / ERR
 */
int pool_init_element_pool(element_pool_t *pool, int entry_size, int entry_count);

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
