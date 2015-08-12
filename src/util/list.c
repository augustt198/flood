#include "list.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void list_new(List *list, int elem_size) {
    list->head      = 0;
    list->tail      = 0;
    list->curr_node = 0;
    list->elem_size = elem_size;
    list->len       = 0;
    pthread_mutex_init(&(list->mutex), NULL);
}

int list_len(List *list) {
    return list->len;
}

bool list_empty(List *list) {
    return list->len == 0;
}

ListNode *create_node(List *list, void *data) {
    ListNode *node = malloc(sizeof(ListNode));
    node->data = malloc(sizeof(list->elem_size));
    memcpy(node->data, data, list->elem_size);

    return node;
}

void list_append(List *list, void *data) {
    pthread_mutex_lock(&(list->mutex));

    ListNode *new_node = create_node(list, data);

    if (list->len == 0) {
        list->head = new_node;
        list->tail = new_node;
        new_node->next = 0;
        new_node->prev = 0;
    } else {
        list->tail->next = new_node;
        new_node->next = 0;
        new_node->prev = list->tail;

        list->tail = new_node;
    }

    list->len++;
    pthread_mutex_unlock(&(list->mutex));
}

void list_prepend(List *list, void *data) {
    pthread_mutex_lock(&(list->mutex));

    ListNode *new_node = create_node(list, data);

    if (list->len == 0) {
        list->head = new_node;
        list->tail = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    } else {
        list->head->prev = new_node;
        new_node->next = list->head;
        new_node->prev = NULL;
        
        list->head = new_node;
    }

    list->len++;

    pthread_mutex_unlock(&(list->mutex));

}


bool list_insert(List *list, int idx, void *data) {
    if (idx < 0 || idx > list->len)
        return false;

    if (idx == list->len) {
        list_append(list, data);
        return true;
    } else if (idx == 0) {
        list_prepend(list, data);
        return true;
    }
    pthread_mutex_lock(&(list->mutex));


    ListNode *new_node = create_node(list, data);
    ListNode *node;
    int mid = list->len / 2;
    if (idx <= mid) {
        node = list->head;
        for (int i = 0; i < idx; i++) {
            node = node->next;
        }
    } else {
        node = list->tail;
        for (int i = 0; i < list->len - idx - 1; i++) {
            node = node->prev;
        }
    }

    node->prev->next = new_node;
    new_node->prev   = node->prev;
    new_node->next   = node;
    node->prev       = new_node;

    list->len++;

    pthread_mutex_unlock(&(list->mutex));
    return true;
}

bool list_truncate(List *list) {
    pthread_mutex_lock(&(list->mutex));
    if (list->len < 1) {
        return false;
    }

    ListNode *del_node = list->tail;
    if (list->len == 1) {
        list->head = 0;
        list->tail = 0;
    } else {
        list->tail = del_node->prev;
        del_node->prev->next = 0;
    }

    free(del_node);
    list->len--;

    pthread_mutex_unlock(&(list->mutex));
    return true;
}

bool list_get(List *list, int idx, void *data) {
    if (idx < 0 || idx >= list->len) {
        return false;
    }
    pthread_mutex_lock(&(list->mutex));

    int mid = list->len / 2;
    ListNode *node;

    if (idx <= mid) { // iterate forward
        node = list->head;
        for (int i = 0; i < idx; i++) {
            node = node->next;
        }
    } else { // iterate backwards
        node = list->tail;
        for (int i = 0; i < list->len - idx - 1; i++) {
            node = node->prev;
        }
    }

    memcpy(data, node->data, list->elem_size);
    pthread_mutex_unlock(&(list->mutex));
    return true;
}

void list_each(List *list, int (*fn)(int, void *)) {
    ListNode *node = list->head;
    for (int i = 0; node; i++) {
        if (fn(i, node->data) != 0)
            break;

        node = node->next;
    }
}

void list_iter_start(List *list) {
    list->curr_node = list->head;
}

bool list_iter_has_next(List *list) {
    return list->curr_node != 0;
}

bool list_iter_next(List *list, void *data) {
    if (list->curr_node == 0) {
        return false;
    }

    memcpy(data, list->curr_node->data, list->elem_size);
    list->curr_node = list->curr_node->next;

    return true;
}
