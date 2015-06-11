/*
 * list.c
 *
 *  Created on: May 24, 2015
 *      Author: liyozhao
 */

#include "list.h"

/**
 * @fn list_pop_front_internal
 *
 * @brief Delete first element.
 * @param [in] list - list pointer
 * @return  - node to be removed
 */
static inline node_t * list_pop_front_internal(list_t *list)
{
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
 * @fn list_clear
 *
 * @brief Removes all elements from the list container.
 * @param [in] list - list pointer
 * @param [in] remove_node_cb - call back function to handle removed node in list
 * @return  - none
 */
void list_clear(list_t *list, void (*remove_node_cb)(node_t *node))
{
    if (unlikely(NULL == list)) {
        DEBUG_ERROR("input parameter %s is null.", "list");
        return;
    }

    if (unlikely(0 == list->total_nodes)) {
        DEBUG_INFO("%s is empty.", "list");
        return;
    }

    node_t *node_to_be_removed = list_pop_front_internal(list);
    while (node_to_be_removed) {
        if (likely(remove_node_cb)) {
            remove_node_cb(node_to_be_removed);
        }
        node_to_be_removed = (0 == list->total_nodes) ? NULL : list_pop_front_internal(list);
    }
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
node_t * list_foreach(list_t *list, int (*traverse_node_cb)(node_t *node))
{
    if (unlikely(NULL == list)) {
        DEBUG_ERROR("input parameter %s is null.", "list");
        return NULL;
    }

    if (unlikely(0 == list->total_nodes)) {
        DEBUG_INFO("%s is empty.", "list");
        return NULL;
    }

    node_t *node_to_be_traversed = list->head_node;
    while (node_to_be_traversed) {
        node_t *next_node_to_be_traversed = node_to_be_traversed->next_node;
        if (likely(traverse_node_cb && (0 == traverse_node_cb(node_to_be_traversed)))) {
            break;
        }
        node_to_be_traversed = next_node_to_be_traversed;
    }
    return node_to_be_traversed;
}

/**
 * @fn list_reverse_foreach
 *
 * @brief reverse traverse node in list
 * @param [in] list - list pointer
 * @param [in] int (*traverse_node_cb)(node_t *node) - call back function to handle traversed node in list;
 *             return  - 0: traversing over; 1: continue traversing
 * @return  node_t * - the node to be found
 */
node_t * list_reverse_foreach(list_t *list, int (*traverse_node_cb)(node_t *node))
{
    if (unlikely(NULL == list)) {
        DEBUG_ERROR("input parameter %s is null.", "list");
        return NULL;
    }

    if (unlikely(0 == list->total_nodes)) {
        DEBUG_INFO("%s is empty.", "list");
        return NULL;
    }

    node_t *node_to_be_traversed = list->tail_node;
    while (node_to_be_traversed) {
        node_t *next_node_to_be_traversed = node_to_be_traversed->previous_node;
        if (likely(traverse_node_cb && (0 == traverse_node_cb(node_to_be_traversed)))) {
            break;
        }
        node_to_be_traversed = next_node_to_be_traversed;
    }
    return node_to_be_traversed;
}

/**
 * @fn list_foreach_with_usr_data
 *
 * @brief traverse node in list
 * @param [in] list - list pointer
 * @param [in] int (*traverse_node_cb)(node_t *node) - call back function to handle traversed node in list;
 *             return  - 0: traversing over; 1: continue traversing
 * @return  node_t * - the node to be found
 */
node_t * list_foreach_with_usr_data(list_t *list, int (*traverse_node_cb)(node_t *node, void* usr_data), void* usr_data)
{
    if (unlikely(NULL == list)) {
        DEBUG_ERROR("input parameter %s is null.", "list");
        return NULL;
    }

    if (unlikely(0 == list->total_nodes)) {
        DEBUG_INFO("%s is empty.", "list");
        return NULL;
    }

    node_t *node_to_be_traversed = list->head_node;
    while (node_to_be_traversed) {
        node_t *next_node_to_be_traversed = node_to_be_traversed->next_node;
        if (likely(traverse_node_cb && (0 == traverse_node_cb(node_to_be_traversed, usr_data)))) {
            break;
        }
        node_to_be_traversed = next_node_to_be_traversed;
    }
    return node_to_be_traversed;
}

