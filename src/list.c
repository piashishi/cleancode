/*
 * list.c
 *
 *  Created on: May 24, 2015
 *      Author: liyozhao
 */

#include "list.h"

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

/**
 * @fn list_init
 *
 * @brief Init list.
 * @param [in] list - list pointer
 * @return - none
 */
void list_init(list_t *list)
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
int list_empty(const list_t *list)
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
int list_size(const list_t *list)
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
node_t* list_front(list_t *list)
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
node_t* list_back(list_t *list)
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
void list_push_front(list_t *list, node_t *node)
{
    if (NULL == list || NULL == node) {
        return;
    }

    if (list_empty(list)) {
        list->head_node = node;
        list->tail_node = node;

        list->head_node->previous_node = NULL;
        list->head_node->next_node = NULL;

        list->tail_node->previous_node = NULL;
        list->tail_node->next_node = NULL;
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
void list_push_back(list_t *list, node_t *node)
{
    if (NULL == list || NULL == node) {
        return;
    }

    if (list_empty(list)) {
        list->head_node = node;
        list->tail_node = node;

        list->head_node->previous_node = NULL;
        list->head_node->next_node = NULL;

        list->tail_node->previous_node = NULL;
        list->tail_node->next_node = NULL;
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
 * @return  - none
 */
void list_remove(list_t *list, node_t *node)
{
    if (NULL == list || NULL == node) {
        return;
    }

    if (list_empty(list)) {
        return;
    }

    node_t *currentNode = list->head_node;
    while (currentNode) {
        if (node != currentNode) {
            currentNode = currentNode->next_node;
            continue;
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
            currentNode->previous_node->next_node = currentNode->next_node;
            currentNode->next_node->previous_node = currentNode->previous_node;
        }

        list->total_nodes--;
        break;
    }
}

/**
 * @fn list_pop_front
 *
 * @brief Delete first element.
 * @param [in] list - list pointer
 * @return  - node to be removed
 */
node_t * list_pop_front(list_t *list)
{
    if (NULL == list) {
        return NULL;
    }

    if (list_empty(list)) {
        return NULL;
    }

    node_t *node_to_be_removed = list->head_node;

    if (1 == list_size(list)) {
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
node_t * list_pop_back(list_t *list)
{
    if (NULL == list) {
        return NULL;
    }

    if (list_empty(list)) {
        return NULL;
    }

    node_t *node_to_be_removed = list->tail_node;

    if (1 == list_size(list)) {
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
 * @fn list_clear
 *
 * @brief Removes all elements from the list container.
 * @param [in] list - list pointer
 * @param [in] remove_node_cb - call back function to handle removed node in list
 * @return  - none
 */
void list_clear(list_t *list, void (*remove_node_cb)(node_t *node))
{
    if (NULL == list) {
        return;
    }

    if (list_empty(list)) {
        return;
    }

    node_t *node_to_be_removed = list_pop_front(list);
    while (node_to_be_removed) {
        if (remove_node_cb) {
            remove_node_cb(node_to_be_removed);
        }
        node_to_be_removed = list_pop_front(list);
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
    if (NULL == list) {
        return NULL;
    }

    if (list_empty(list)) {
        return NULL;
    }

    node_t *node_to_be_traversed = list->head_node;
    while (node_to_be_traversed) {
        if (traverse_node_cb) {
            if (0 == traverse_node_cb(node_to_be_traversed)) {
                break;
            }
        }
        node_to_be_traversed = node_to_be_traversed->next_node;
    }
    return node_to_be_traversed;
}

