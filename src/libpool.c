#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "libpool.h"

#define INVALID_INDEX (-1)
#define MAGIC_CHECK_VALUE (13)

typedef int pools_count_t ;

static size_t pool_caculate_pool_head_length(void)
{
    return sizeof(element_pool_t);
}

static size_t pool_caculate_nodes_length(int entry_acount)
{
    return sizeof(node_t) * entry_acount;
}

static size_t pool_caculate_element_length(size_t entry_size)
{
    while ((entry_size) % 4 != 0) {
        (entry_size) = (entry_size) + 1;
    }
    return sizeof(element_usr_data_t) + entry_size;
}
static size_t pool_caculate_elements_length(size_t entry_size, int entry_acount)
{
    return pool_caculate_element_length(entry_size) * entry_acount;
}

static element_pool_t* get_pool_ctrl(void* pools, int index)
{
    pools_count_t* pool_count =(pools_count_t*)pools;
    if (*pool_count < index) {
        DEBUG_ERROR("invalid index while get pool, index = %d", index);
        return NULL;
    }

    element_pool_t** pools_pointer = (element_pool_t**)((char *)pools + sizeof(pools_count_t));
    return pools_pointer[index];
}

static size_t pool_caculate_length(size_t entry_size, int entry_acount)
{
    size_t pool_head_length = pool_caculate_pool_head_length();
    size_t nodes_length = pool_caculate_nodes_length(entry_acount);
    size_t elements_length = pool_caculate_elements_length(entry_size, entry_acount);

    return pool_head_length + nodes_length + elements_length;
}

size_t pool_caculate_total_length(int pool_acount, pool_attr_t pool_attr[])
{
    size_t pools_head_size = sizeof(pools_count_t) + sizeof(element_pool_t*) * pool_acount;
    int i;
    size_t pools_length = 0;
    for (i = 0; i < pool_acount; i++) {
        size_t pool_length = pool_caculate_length(pool_attr[i].entry_size, pool_attr[i].entry_acount);
        pools_length = pools_length + pool_length;
    }

    return pools_head_size + pools_length;
}
static node_t* pool_get_node_addr(element_pool_t* pool, int j)
{
    node_t* nodes_start_mem = (node_t*)((char*) pool + pool_caculate_pool_head_length());
    return nodes_start_mem + j;
}

static element_usr_data_t* pool_get_element_addr(element_pool_t* pool, int j)
{
    element_usr_data_t* elements_start_mem = (element_usr_data_t*) ((char*) pool + pool_caculate_pool_head_length()
            + pool_caculate_nodes_length(pool->element_acount));

    return (element_usr_data_t*) ((char*) elements_start_mem + pool->element_size * j);
}

// | pools_count_t | element_pool_t* pools[ 0, 1, ... ] |
// | element_pool_t pools 0 | + | node_t 0.0 | node 0.1 | ... | + | element_usr_data_t 0.0 | entry_0.0 | ... |
// | element_pool_t pools 1 | + | node_t 1.0 | node 1.1 | ... | + | element_usr_data_t 1.0 | entry_1.0 | ... |
// | ... |
void* pools_init(void* large_memory, size_t large_mem_size, int pool_acount, pool_attr_t pool_attr[])
{
    size_t total_length = pool_caculate_total_length(pool_acount, pool_attr);
    if (large_mem_size < total_length) {
        return NULL;
    }

    pools_count_t *pools_count = (pools_count_t*)large_memory;
    *pools_count = pool_acount;

    element_pool_t** pool_pointers = (element_pool_t**) ((char*) large_memory + sizeof(pools_count_t));
    element_pool_t* pool = (element_pool_t*) ((char*) large_memory + sizeof(pools_count_t)
            + sizeof(element_pool_t*) * pool_acount);
    int i;
    for (i = 0; i < pool_acount; i++) {
        pool_pointers[i] = pool;

        memset(pool, '\0', sizeof(element_pool_t));

        list_init(&pool->free_list);
        list_init(&pool->busy_list);

        pool->element_size = pool_caculate_element_length(pool_attr[i].entry_size);
        pool->element_acount = pool_attr[i].entry_acount;

        int j;
        for (j = 0; j < pool->element_acount; j++) {
            node_t* node = pool_get_node_addr(pool, j);
            element_usr_data_t* elements_addr = pool_get_element_addr(pool, j);

            elements_addr->check_value = MAGIC_CHECK_VALUE;
            elements_addr->reserved_pointer = NULL;
            elements_addr->to_node = node;

            node->usr_data = (void*)(elements_addr + 1); //

            list_push_back(&pool->free_list, node);
        }

        size_t pool_length = pool_caculate_length(pool_attr[i].entry_size, pool_attr[i].entry_acount);
        pool = (element_pool_t*)((char*)pool + pool_length);
    }

    return (element_pool_t**) large_memory;
}


void* pool_get_element(void* pools, int pool_type)
{
    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    if (pool == NULL) {
        return NULL;
    }

    node_t *node = list_pop_back(&pool->free_list);
    void* element;
    if (node == NULL) {
        DEBUG_INFO("Pool have no more free %s.", "node");
        element = NULL;
    } else {
        list_push_back(&pool->busy_list, node);
        element = node->usr_data;
    }

    return element;
}

static void* pool_get_element_head(void* element)
{
    if ((char*)element - (char*)NULL <= sizeof(element_usr_data_t)) {
        DEBUG_ERROR("Element is invalid, element = %p.", element);
        return NULL;
    }

    void* element_head;

    element_usr_data_t *element_user_data = (element_usr_data_t *) ((char*) element - sizeof(element_usr_data_t));
    if (element_user_data->check_value != MAGIC_CHECK_VALUE) {
        DEBUG_ERROR("Element is a invalid to free, check_value = %d.", element_user_data->check_value);
        element_head = NULL;
    } else {
        element_head = element_user_data;
    }

    return element_head;
}

return_t pool_free_element(void *pools, int pool_type, void* element)
{
    element_usr_data_t *element_user_data = pool_get_element_head(element);
    if (element_user_data == NULL) {
        DEBUG_ERROR("Element address get a NULL %s", "element_user_data");
        return ERR;
    }

    element_user_data->reserved_pointer = NULL;

    node_t* node = element_user_data->to_node;
    if (node == NULL) {
        DEBUG_ERROR("Can't find the %s of element address", "node");
        return ERR;
    }

    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    list_remove(&pool->busy_list, node);
    list_push_back(&pool->free_list, node);

    return OK;
}

return_t pool_set_reserved_pointer(void* element, void* to_set)
{
    return_t ret;
    element_usr_data_t *element_user_data = (element_usr_data_t *) pool_get_element_head(element);
    if (element_user_data == NULL) {
        DEBUG_ERROR("%s is NULL.", "element_user_data");
        ret = ERR;
    } else {
        element_user_data->reserved_pointer = to_set;
        ret = OK;
    }

    return ret;
}

void* pool_get_reserved_pointer(void* element)
{
    void *reserved_pointer;
    element_usr_data_t *element_user_data = pool_get_element_head(element);
    if (element_user_data == NULL) {
        DEBUG_ERROR("%s is NULL.", "element_user_data");
        reserved_pointer = NULL;
    } else {
        reserved_pointer = element_user_data->reserved_pointer;
    }

    return reserved_pointer;
}

