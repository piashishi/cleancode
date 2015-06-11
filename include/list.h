/*
 * list.h
 *
 *  Created on: May 24, 2015
 *      Author: liyozhao
 */

#ifndef LIBLIST_H_
#define LIBLIST_H_

#include <string.h>
#include <stdio.h>
#include "libcache_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct node_t
{
    void *usr_data;
    struct node_t *previous_node; /* pointer to previous child */
    struct node_t *next_node; /* pointer to next child */
}__attribute__((aligned(8))) node_t;

typedef struct list_t
{
    unsigned int total_nodes;
    node_t *head_node;
    node_t *tail_node;
} __attribute__((aligned(8))) list_t;


/**
 * @fn list_init
 *
 * @brief Init list.
 * @param [in] list - list pointer
 * @return - none
 */
static inline void list_init(list_t *list)
{
    if (NULL == list) {
        return;
    }
    list->total_nodes = 0;
    list->head_node = NULL;
    list->tail_node = NULL;
}


/**
 * @fn list_size
 *
 * @brief Judge whether the list is empty or not.
 * @param [in] list - list pointer
 * @return 0  - list is not empty
 * @return 1  - list is empty
 */
static inline int list_empty(const list_t *list)
{
    return (NULL == list) ? TRUE : (0 == list->total_nodes);
}

/**
 * @fn list_size
 *
 * @brief Returns the number of elements in the list.
 * @param [in] list - list pointer
 * @return int  - total nodes in the list
 */
static inline int list_size(const list_t *list)
{
    return (NULL == list) ? 0 : list->total_nodes;
}

/**
 * @fn list_front
 *
 * @brief Returns the first elements in the list.
 * @param [in] list - list pointer
 * @return node_t*  - the first elements reference in the list
 */
static inline node_t* list_front(list_t *list)
{
    return (NULL == list) ? NULL : list->head_node;
}

/**
 * @fn list_back
 *
 * @brief Returns the last elements in the list.
 * @param [in] list - list pointer
 * @return node_t*  - the last elements reference in the list
 */
static inline node_t* list_back(list_t *list)
{
    return (NULL == list) ? NULL : list->tail_node;
}

/**
 * @fn list_push_front
 *
 * @brief Insert element at beginning.
 * @param [in] list - list pointer
 * @param [in] node - node to be inserted
 * @return  - none
 */
//void list_push_front(list_t *list, node_t *node);
static inline void list_push_front(list_t *list, node_t *node)
{
    if (NULL == list || NULL == node) {
        DEBUG_ERROR("input parameter %s %s is null.", (NULL == list) ? "list" : "", (NULL == node) ? "node" : "");
        return;
    }

    if (0 == list->total_nodes) {
        node->previous_node = NULL;
        node->next_node = NULL;
        list->head_node = node;
        list->tail_node = node;
    } else {
        list->head_node->previous_node = node;
        node->next_node = list->head_node;
        node->previous_node = NULL;
        list->head_node = node;
    }

    list->total_nodes++;
}

/**
 * @fn list_push_back
 *
 * @brief Insert element at back.
 * @param [in] list - list pointer
 * @param [in] node - node to be inserted
 * @return  - none
 */
static inline void list_push_back(list_t *list, node_t *node)
{
    if (NULL == list || NULL == node) {
        DEBUG_ERROR("input parameter %s %s is null.", (NULL == list) ? "list" : "", (NULL == node) ? "node" : "");
        return;
    }
    if (0 == list->total_nodes) {
        node->previous_node = NULL;
        node->next_node = NULL;
        list->head_node = node;
        list->tail_node = node;

    } else {
        list->tail_node->next_node = node;
        node->previous_node = list->tail_node;
        node->next_node = NULL;
        list->tail_node = node;
    }

    list->total_nodes++;
}

/**
 * @fn list_remove
 *
 * @brief remove element.
 * @param [in] list - list pointer
 * @param [in] node - node to be removed
 * @return  int - TRUE: removed; FALSE: error or node not found
 */
/**
 * @fn list_remove
 *
 * @brief remove element.
 * @param [in] list - list pointer
 * @param [in] node - node to be removed
 * @return  int - TRUE: removed; FALSE: error or node not found
 */
static inline int list_remove(list_t *list, node_t *node)
{
    if (NULL == list || NULL == node) {
        DEBUG_ERROR("input parameter %s %s is null.", (NULL == list) ? "list" : "", (NULL == node) ? "node" : "");
        return FALSE;
    }

    if (0 == list->total_nodes) {
        DEBUG_INFO("%s is empty.", "list");
        return FALSE;
    }

    if (node == list->head_node) {
        if (list->head_node->next_node) {
            list->head_node->next_node->previous_node = NULL;
            list->head_node = list->head_node->next_node;
        } else {
            list->head_node = NULL;
            list->tail_node = NULL;
        }
    } else if (node == list->tail_node) {
        if (list->tail_node->previous_node) {
            list->tail_node->previous_node->next_node = NULL;
            list->tail_node = list->tail_node->previous_node;
        } else {
            list->head_node = NULL;
            list->tail_node = NULL;
        }
    } else {
        node->previous_node->next_node = node->next_node;
        node->next_node->previous_node = node->previous_node;
    }

    list->total_nodes--;

    return TRUE;
}

/**
 * @fn list_pop_front
 *
 * @brief Delete first element.
 * @param [in] list - list pointer
 * @return  - node to be removed
 */
static inline node_t * list_pop_front(list_t *list)
{
    if (NULL == list) {
        DEBUG_ERROR("input parameter %s is null.", "list");
        return NULL;
    }

    if (0 == list->total_nodes) {
        DEBUG_INFO("%s is empty.", "list");
        return NULL;
    }

    node_t *node_to_be_removed = list->head_node;

    if (1 == list->total_nodes) {
        list->head_node = NULL;
        list->tail_node = NULL;
    } else {
        list->head_node->next_node->previous_node = NULL;
        list->head_node = list->head_node->next_node;
    }

    list->total_nodes--;

    return node_to_be_removed;
}

/**
 * @fn list_pop_back
 *
 * @brief Delete last element.
 * @param [in] list - list pointer
 * @return  - node to be removed
 */
//node_t * list_pop_back(list_t *list);
static inline node_t * list_pop_back(list_t *list)
{
    if (NULL == list) {
        DEBUG_ERROR("input parameter %s is null.", "list");
        return NULL;
    }

    if (0 == list->total_nodes) {
        DEBUG_INFO("%s is empty.", "list");
        return NULL;
    }

    node_t *node_to_be_removed = list->tail_node;

    if (1 == list->total_nodes) {
        list->head_node = NULL;
        list->tail_node = NULL;
    } else {
        list->tail_node->previous_node->next_node = NULL;
        list->tail_node = list->tail_node->previous_node;
    }

    list->total_nodes--;

    return node_to_be_removed;
}

/**
 * @fn list_foreach
 *
 * @brief traverse node in list
 * @param [in] list - list pointer
 * @param [in] int (*traverse_node_cb)(node_t *node) - call back function to handle traversed node in list;
 *             return  - 0: traversing over; 1: continue traversing
 * @return  node_t * - the node to be found
 */
node_t * list_foreach(list_t *list, int (*traverse_node_cb)(node_t *node));

/**
 * @fn list_foreach_with_usr_data
 *
 * @brief traverse node in list
 * @param [in] list - list pointer
 * @param [in] int (*traverse_node_cb)(node_t *node) - call back function to handle traversed node in list;
 *             return  - 0: traversing over; 1: continue traversing
 * @return  node_t * - the node to be found
 */
node_t * list_foreach_with_usr_data(list_t *list, int (*traverse_node_cb)(node_t *node, void* usr_data), void* usr_data);

/**
 * @fn list_clear
 *
 * @brief Removes all elements from the list container.
 * @param [in] list - list pointer
 * @param [in] remove_node_cb - call back function to handle removed node in list
 * @return  - none
 */
void list_clear(list_t *list, void (*remove_node_cb)(node_t *node));

/**
 * @fn list_reverse_foreach
 *
 * @brief reverse traverse node in list
 * @param [in] list - list pointer
 * @param [in] int (*traverse_node_cb)(node_t *node) - call back function to handle traversed node in list;
 *             return  - 0: traversing over; 1: continue traversing
 * @return  node_t * - the node to be found
 */
node_t * list_reverse_foreach(list_t *list, int (*traverse_node_cb)(node_t *node));

/**
 * @fn list_swap_to_head
 *
 * @brief swap element.
 * @param [in] list - list pointer
 * @param [in] node - node to be removed
 * @return
 */
static inline void list_swap_to_head(list_t *list, node_t *node)
{
    if (1 == list->total_nodes) {
        return;
    } else if (node == list->head_node) {
        return;
    } else if (node == list->tail_node) {
        node->previous_node->next_node = NULL;
        list->tail_node = node->previous_node;
        node->next_node = list->head_node;
        list->head_node->previous_node = node;
        node->previous_node = NULL;
        list->head_node = node;
    } else {
        node->previous_node->next_node = node->next_node;
        node->next_node->previous_node = node->previous_node;

        node->next_node = list->head_node;
        list->head_node->previous_node = node;
        node->previous_node = NULL;
        list->head_node = node;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* LIBLIST_H_ */
