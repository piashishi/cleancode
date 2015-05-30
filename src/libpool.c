#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "libpool.h"

#define INVALID_INDEX (-1)

static void pool_cb_register(element_pool_t* pool,
                      LIBCACHE_ALLOCATE_MEMORY* allocate_memory,
                      LIBCACHE_FREE_MEMORY* free_memory)
{
    if (allocate_memory == NULL) {
        pool->cb.allocate_memory = malloc;
    } else {
        pool->cb.allocate_memory = allocate_memory;
    }

    if (free_memory == NULL) {
        pool->cb.free_memory = free;
    } else {
        pool->cb.free_memory = free_memory;
    }
}

element_pool_t* pool_init(size_t size,
                          LIBCACHE_ALLOCATE_MEMORY* allocate_memory,
                          LIBCACHE_FREE_MEMORY* free_memory)
{
    LIBCACHE_ALLOCATE_MEMORY* allocate_func;
    element_pool_t *pool;
    if (allocate_memory == NULL) {
        pool = (element_pool_t*) malloc(sizeof (element_pool_t));
    } else {
        pool = (element_pool_t*) allocate_memory(sizeof (element_pool_t));
    }

    if (pool == NULL) {
        return NULL;
    }

    memset(pool, '\0', sizeof(pool));
    list_init(&pool->free_list);
    list_init(&pool->busy_list);
    pool_cb_register(pool, allocate_memory, free_memory);

    while (size % 4 != 0) {
        size++;
    }

    pool->start_memory = (void*) pool->cb.allocate_memory(size);
    if (pool->start_memory == NULL) {
        pool->cb.free_memory(pool);
        return NULL;
    }

    pool->memory_size = size;

    return pool;
}

static int get_index(element_pool_t *pool, void *element_addr)
{
    if ((((char*) element_addr - (char*) pool->start_memory) < 0)
            || ((char*) element_addr - (char*) pool->start_memory) > pool->memory_size) {
        return INVALID_INDEX;
    }

    int index = ((char*) element_addr - (char*) pool->start_memory) / pool->element_size;
    return index;
}

int pool_init_element_pool(element_pool_t *pool, size_t entry_size, int entry_count)
{
    while (entry_size % 4 != 0) {
        entry_size++;
    }

    if (entry_size * entry_count > pool->memory_size) {
        return ERR;
    }

    pool->element_link = (node_t**) pool->cb.allocate_memory(sizeof(node_t**) * entry_count);
    if (pool->element_link == NULL) {
        return ERR;
    }
    memset(pool->element_link, '\0', sizeof(node_t*) * entry_count);

    pool->element_size = entry_size;

    void* addr = pool->start_memory;
    int i;
    for (i = 0; i < entry_count; i++) {
        node_t* node = (node_t*) pool->cb.allocate_memory(sizeof(node_t));
        if (node == NULL) {
            return ERR;
        }

        node->usr_data = addr;

        list_push_back(&pool->free_list, node);

        int index = get_index(pool, addr);
        pool->element_link[index] = node;

        addr = (void*) ((char*) addr + entry_size);
    }

    return OK;
}

void* pool_get_element(element_pool_t *pool)
{
    node_t *node = list_pop_back(&pool->free_list);
    if (node == NULL) {
        return NULL;
    } else {
        list_push_back(&pool->busy_list, node);
        return node->usr_data;
    }
}

int pool_free_element(element_pool_t *pool, void* element)
{
    int index = get_index(pool, element);
    if (index == INVALID_INDEX) {
        return ERR;
    }

    node_t *node = pool->element_link[index];
    if (node == NULL) {
        return ERR;
    }

    pool->element_link[index] = NULL;

    list_remove(&pool->busy_list, node);
    list_push_back(&pool->free_list, node);

    return OK;
}

