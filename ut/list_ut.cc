#include "UnitTest++.h"
#include "list.h"

TEST(list_ut)
{
    node_t node_1;
    memset(&node_1, 0, sizeof(node_t));

    node_t node_2;
    memset(&node_2, 0, sizeof(node_t));

    node_t node_3;
    memset(&node_3, 0, sizeof(node_t));

    node_t node_4;
    memset(&node_4, 0, sizeof(node_t));

    node_t node_5;
    memset(&node_5, 0, sizeof(node_t));

    list_t list;
    list_init(&list);
    list_push_front(&list, &node_1);
    list_push_front(&list, &node_2);
    list_push_front(&list, &node_3);
    list_push_back(&list, &node_4);
    list_push_back(&list, &node_5);
    CHECK_EQUAL(5, list_size(&list));

    list_remove(&list, &node_1);
    list_remove(&list, &node_2);
    CHECK_EQUAL(3, list_size(&list));

    list_pop_back(&list);
    list_pop_front(&list);
    CHECK_EQUAL(0, list_empty(&list));
    CHECK_EQUAL(1, list_size(&list));
    CHECK_EQUAL(1, NULL != list_front(&list));
    CHECK_EQUAL(1, NULL != list_back(&list));

    list_clear(&list, NULL);
    CHECK_EQUAL(0, list_size(&list));
    CHECK_EQUAL(1, NULL == list_foreach(&list, NULL));
}
