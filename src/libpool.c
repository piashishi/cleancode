#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "libpool.h"

#define INVALID_INDEX (-1)

#define ALIGN(d_num)           \
do {                           \
    while ((d_num) % 4 != 0) { \
        (d_num) = (d_num) + 1; \
    }                          \
} while (0);

element_pool_t* get_pool_ctrl(void* pool_ctrl_start_memory, int index)
{
    return (element_pool_t*) ((char*) pool_ctrl_start_memory + sizeof(element_pool_t) * index);
}

element_pool_t* pools_init(void* large_memory, size_t large_mem_size, pool_type_e pool_acount, pool_attr_t pool_attr[])
{
    int i;
    size_t pool_ctrl_length = sizeof(element_pool_t) * pool_acount;
    ALIGN(pool_ctrl_length);
    int length = pool_ctrl_length;
    for (i = 0; i < pool_acount; i++) {
        pool_attr[i].entry_size = pool_attr[i].entry_size + sizeof(element_usr_data_t);
        ALIGN(pool_attr[i].entry_size);

        int entrys_length = pool_attr[i].entry_size * pool_attr[i].entry_acount;
        ALIGN(entrys_length);

        length = length + entrys_length;
    }

    if (large_mem_size < length) {
        return NULL;
    }

    char* tmp_memory = (char*)large_memory + pool_ctrl_length;
    for (i = 0; i < pool_acount; i++) {
        element_pool_t* pool = get_pool_ctrl(large_memory, i);
        memset(pool, '\0', sizeof(element_pool_t));

        list_init(&pool->free_list);
        list_init(&pool->busy_list);

        pool->start_memory = (void*)tmp_memory;

        int entrys_length = pool_attr[i].entry_size * pool_attr[i].entry_acount;
        ALIGN(entrys_length);

        pool->memory_size = entrys_length;
        pool->element_size = pool_attr[i].entry_size;
        pool->element_link = NULL; //TODO

        tmp_memory = tmp_memory + entrys_length;
    }

    for (i = 0; i < pool_acount; i++) {
        element_pool_t* pool = get_pool_ctrl(large_memory, i);
        int entry_count = pool_attr[i].entry_acount;
        size_t entry_size = pool_attr[i].entry_size;

        element_usr_data_t* addr = (element_usr_data_t*)pool->start_memory;
        int j;
        for (j = 0; j < entry_count; j++) {
            node_t* node = (node_t*) malloc(sizeof(node_t));
            if (node == NULL) {
                return NULL;
            }

            addr->key = NULL;
            addr->to_node = node;

            node->usr_data = (void*)((char*)addr + sizeof(element_usr_data_t)); //

            list_push_back(&pool->free_list, node);

            addr = (void*) ((char*) addr + entry_size);
        }
    }

    return (element_pool_t*)large_memory;

}


void* pool_get_element(element_pool_t *pools, pool_type_e pool_type)
{
    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    node_t *node = list_pop_back(&pool->free_list);
    if (node == NULL) {
        return NULL;
    } else {
        list_push_back(&pool->busy_list, node);
        return node->usr_data;
    }
}

int pool_free_element(element_pool_t *pools, pool_type_e pool_type, void* element)
{
    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    if ((((char*) element - (char*) pool->start_memory) < 0)
            || ((char*) element - (char*) pool->start_memory) > pool->memory_size) {
        printf("*** pool_free_element invalid element address.\n");
        return ERR;
    }

    element_usr_data_t *element_user_data = (element_usr_data_t *) ((char*) element - sizeof(element_usr_data_t)); // TODO
    if (element_user_data == NULL) {
        printf("*** pool_free_element element_user_data == NULL.\n");
        return ERR;
    }

    node_t* node = element_user_data->to_node;
    if (node == NULL) {
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

void** pool_get_key_address_by_element_address(element_pool_t *pools, pool_type_e pool_type, void* element)
{
    element_pool_t *pool = get_pool_ctrl(pools, pool_type);
    if ((((char*) element - (char*) pool->start_memory) < 0)
            || ((char*) element - (char*) pool->start_memory) > pool->memory_size) {
        printf("*** pool_free_element invalid element address.\n");
        return NULL;
    }

    element_usr_data_t *element_user_data = (element_usr_data_t *) ((char*) element - sizeof(element_usr_data_t)); // TODO
    if (element_user_data == NULL) {
        printf("*** pool_free_element element_user_data == NULL.\n");
        return NULL;
    }

    return &element_user_data->key;
}
