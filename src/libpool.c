#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "libpool.h"

#define INVALID_INDEX (-1)
#define MAGIC_CHECK_VALUE (13)

#define ALIGN(d_num)           \
do {                           \
    while ((d_num) % 4 != 0) { \
        (d_num) = (d_num) + 1; \
    }                          \
} while (0);

typedef int pools_count_t ;

size_t pool_caculate_pool_head_length()
{
    return sizeof(element_pool_t);
}

size_t pool_caculate_nodes_length(int entry_acount)
{
    return sizeof(node_t) * entry_acount;
}

size_t pool_caculate_element_length(size_t entry_size)
{
    ALIGN(entry_size);
    return sizeof(element_usr_data_t) + entry_size;
}
size_t pool_caculate_elements_length(size_t entry_size, int entry_acount)
{
    return pool_caculate_element_length(entry_size) * entry_acount;
}

element_pool_t* get_pool_ctrl(void* pools, int index)
{
    pools_count_t* pool_count =(pools_count_t*)pools;
    if (*pool_count < index) {
        printf("get_pool_ctrl ERROR!!! \n");
        return NULL;
    }

    element_pool_t** pools_pointer = (element_pool_t**)((char *)pools + sizeof(pools_count_t));
    return pools_pointer[index];
}

size_t pool_caculate_length(size_t entry_size, int entry_acount)
{
    size_t pool_head_length = pool_caculate_pool_head_length();
    size_t nodes_length = pool_caculate_nodes_length(entry_acount);
    size_t elements_length = pool_caculate_elements_length(entry_size, entry_acount);

    return pool_head_length + nodes_length + elements_length;
}

size_t pool_caculate_total_length(pool_type_e pool_acount, pool_attr_t pool_attr[])
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
node_t* pool_get_node_addr(element_pool_t* pool, int j)
{
    node_t* nodes_start_mem = (node_t*)((char*) pool + pool_caculate_pool_head_length());
    return nodes_start_mem + j;
}

element_usr_data_t* pool_get_element_addr(element_pool_t* pool, int j)
{
    element_usr_data_t* elements_start_mem = (element_usr_data_t*) ((char*) pool + pool_caculate_pool_head_length()
            + pool_caculate_nodes_length(pool->element_acount));

    return (element_usr_data_t*) ((char*) elements_start_mem + pool->element_size * j);
}

// | pools_count_t | element_pool_t* pools[ 0, 1, ... ] |
// | element_pool_t pools 0 | + | node_t 0.0 | node 0.1 | ... | + | element_usr_data_t 0.0 | entry_0.0 | ... |
// | element_pool_t pools 1 | + | node_t 1.0 | node 1.1 | ... | + | element_usr_data_t 1.0 | entry_1.0 | ... |
// | ... |
void* pools_init(void* large_memory, size_t large_mem_size, pool_type_e pool_acount, pool_attr_t pool_attr[])
{
    printf("poolp_init - large_memory = %X \n", large_memory);

    size_t total_length = pool_caculate_total_length(pool_acount, pool_attr);
    if (large_mem_size < total_length) {
        return NULL;
    }

    pools_count_t *pools_count = (pools_count_t*)large_memory;
    *pools_count = pool_acount;

    element_pool_t** pool_pointers = (element_pool_t**) ((char*) large_memory + sizeof(pools_count_t));
    element_pool_t* pool = (element_pool_t*) ((char*) large_memory + sizeof(pools_count_t)
            + sizeof(element_pool_t*) * pool_acount);
    printf("poolp_init - pool_pointers = %X \n", pool_pointers);
    int i;
    for (i = 0; i < pool_acount; i++) {
        printf("poolp_init - pool[%d]          = %X \n", i, pool);
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
            printf("poolp_init - node[%d]          = %X \n", j, node);
            printf("poolp_init - elements_addr[%d] = %X \n", j, elements_addr);

            elements_addr->check_value = MAGIC_CHECK_VALUE;
            elements_addr->key = NULL;
            elements_addr->to_node = node;

            node->usr_data = (void*)(elements_addr + 1); //

            list_push_back(&pool->free_list, node);
        }

        size_t pool_length = pool_caculate_length(pool_attr[i].entry_size, pool_attr[i].entry_acount);
        pool = (element_pool_t*)((char*)pool + pool_length);
    }

    return (element_pool_t**) large_memory;

    printf("\n");
}


void* pool_get_element(void* pools, pool_type_e pool_type)
{
    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    printf("pool_get_element - pools = %X \n", pools);
    printf("pool_get_element - pool  = %X \n", pool);

    node_t *node = list_pop_back(&pool->free_list);
    printf("pool_get_element - node  = %X \n", node);
    if (node == NULL) {
        return NULL;
    } else {
        list_push_back(&pool->busy_list, node);
        return node->usr_data;
    }
}

int pool_free_element(void *pools, pool_type_e pool_type, void* element)
{
    if (element == NULL) {
        printf("ERROR: pool_free_element but element is NULL.\n", pools);
        return ERR;
    }

    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    printf("pool_free_element - pools = %X \n", pools);
    printf("pool_free_element - pool  = %X \n", pool);

    element_usr_data_t *element_user_data = (element_usr_data_t *) ((char*) element - sizeof(element_usr_data_t));
    printf("pool_free_element - element  = %X \n", element);
    printf("pool_free_element - element_user_data  = %X \n", element_user_data);
    if (element_user_data == NULL) {
        printf("ERROR: element address get a NULL element_user_data, file: %s, line: %d, function: %s\n",
               __FILE__,
               __LINE__,
               __FUNCTION__);
        return ERR;
    } else if (element_user_data->check_value != MAGIC_CHECK_VALUE) {
        printf("pool_free_element check_value wrong!!!");
        return ERR;
    }

    node_t* node = element_user_data->to_node;
    if (node == NULL) {
        printf("ERROR: cannot find the node of element address, file: %s, line: %d, function: %s\n",
               __FILE__,
               __LINE__,
               __FUNCTION__);
        return ERR;
    }

    if (((element_usr_data_t*) node->usr_data)->key != NULL) {
        free(((element_usr_data_t*) node->usr_data)->key);
    }
    ((element_usr_data_t*) node->usr_data)->key = NULL;

    list_remove(&pool->busy_list, node);
    list_push_back(&pool->free_list, node);

    return OK;
}

void** pool_get_key_address_by_element_address(void* pools, pool_type_e pool_type, void* element)
{
    element_pool_t *pool = get_pool_ctrl(pools, pool_type);

    element_usr_data_t *element_user_data = (element_usr_data_t *) ((char*) element - sizeof(element_usr_data_t)); // TODO
    if (element_user_data == NULL) {
        printf("*** pool_free_element element_user_data == NULL.\n");
        return NULL;
    } else if (element_user_data->check_value != MAGIC_CHECK_VALUE) {
        printf("pool_get_key_address_by_element_address check_value wrong!!!");
        return NULL;
    }

    return &element_user_data->key;
}
