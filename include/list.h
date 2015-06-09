/*
 * list.h
 *
 *  Created on: May 24, 2015
 *      Author: liyozhao
 */

#ifndef LIBLIST_H_
#define LIBLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TRUE  1
#define FALSE 0

typedef struct node_t
{
    void *usr_data;
    struct node_t *previous_node; /* pointer to previous child */
    struct node_t *next_node; /* pointer to next child */
}node_t;

typedef struct list_t
{
    unsigned int total_nodes;
    node_t *head_node;
    node_t *tail_node;
}list_t;


/**
 * @fn list_init
 *
 * @brief Init list.
 * @param [in] list - list pointer
 * @return - none
 */
void list_init(list_t *list);


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
void list_push_front(list_t *list, node_t *node);

/**
 * @fn list_push_back
 *
 * @brief Insert element at back.
 * @param [in] list - list pointer
 * @param [in] node - node to be inserted
 * @return  - none
 */
void list_push_back(list_t *list, node_t *node);

/**
 * @fn list_remove
 *
 * @brief remove element.
 * @param [in] list - list pointer
 * @param [in] node - node to be removed
 * @return  int - TRUE: removed; FALSE: error or node not found
 */
int list_remove(list_t *list, node_t *node);

/**
 * @fn list_pop_front
 *
 * @brief Delete first element.
 * @param [in] list - list pointer
 * @return  - node to be removed
 */
node_t * list_pop_front(list_t *list);

/**
 * @fn list_pop_back
 *
 * @brief Delete last element.
 * @param [in] list - list pointer
 * @return  - node to be removed
 */
node_t * list_pop_back(list_t *list);

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

#ifdef __cplusplus
}
#endif

#endif /* LIBLIST_H_ */
