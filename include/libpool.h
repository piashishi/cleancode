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

element_pool_t pool;

typedef enum return_t {
    OK, ERR
} return_t;

/**
 * @fn pool_init
 *
 * @brief Init memory to pool.
 * @param [in] size - pool size
 * @return -  OK / ERR
 */
int pool_init(int size);

/**
 * @fn pool_init_element_pool
 *
 * @brief Init element pool.
 * @param [in] entry_size  - entry size
 * @param [in] entry_count - entry count
 * @return -  OK / ERR
 */
int pool_init_element_pool(int entry_size, int entry_count);

/**
 * @fn pool_get_element
 *
 * @brief get an unused element memory.
 * @return -  a point to element memory (NULL for failed)
 */
void* pool_get_element();

node_t* get_real_node_addr(void* element); // TODO
/**
 * @fn pool_free_element
 *
 * @brief free an used element memory.
 * @param [in] element  - element address
 * @return -  OK / ERR
 */
int pool_free_element(void* element);

#ifdef __cplusplus
}
#endif
#endif /* LIBPOOL_H_ */