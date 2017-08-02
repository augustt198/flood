#include "list.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void list_new(list_t *list, int elem_size) {
    list->head      = NULL;
    list->tail      = NULL;
    list->curr_node = NULL;
    list->elem_size = elem_size;
    list->len       = 0;
    pthread_mutex_init(&(list->mutex), NULL);
}

int list_len(list_t *list) {
    return list->len;
}

bool list_empty(list_t *list) {
    return list->len == 0;
}

list_node *create_node(list_t *list, void *data) {
    list_node *node = malloc(sizeof(list_node));
    node->data = malloc(list->elem_size);
    node->prev = NULL;
    node->next = NULL;
    memcpy(node->data, data, list->elem_size);

    return node;
}

void list_append(list_t *list, void *data) {
    pthread_mutex_lock(&(list->mutex));

    list_node *new_node = create_node(list, data);

    if (list->len == 0) {
        list->head = new_node;
        list->tail = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    } else {
        list->tail->next = new_node;
        new_node->next = NULL;
        new_node->prev = list->tail;

        list->tail = new_node;
    }

    list->len++;
    pthread_mutex_unlock(&(list->mutex));
}

void list_prepend(list_t *list, void *data) {
    pthread_mutex_lock(&(list->mutex));

    list_node *new_node = create_node(list, data);

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


bool list_insert(list_t *list, int idx, void *data) {
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


    list_node *new_node = create_node(list, data);
    list_node *node;
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

bool list_truncate(list_t *list) {
    pthread_mutex_lock(&(list->mutex));
    if (list->len < 1) {
        return false;
    }

    list_node *del_node = list->tail;
    if (list->len == 1) {
        list->head = NULL;
        list->tail = NULL;
    } else {
        list->tail = del_node->prev;
        del_node->prev->next = NULL;
    }

    free(del_node);
    list->len--;

    pthread_mutex_unlock(&(list->mutex));
    return true;
}

bool list_get(list_t *list, int idx, void *data) {
    if (idx < 0 || idx >= list->len) {
        return false;
    }
    pthread_mutex_lock(&(list->mutex));

    int mid = list->len / 2;
    list_node *node;

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

void list_each(list_t *list, int (*fn)(int, void *)) {
    list_node *node = list->head;
    for (int i = 0; node; i++) {
        if (fn(i, node->data) != 0)
            break;

        node = node->next;
    }
}

void list_iter_start(list_t *list) {
    list->curr_node = list->head;
}

void list_iter_start_safe(list_t *list) {
    pthread_mutex_lock(&(list->mutex));
    list->curr_node = list->head;
}

bool list_iter_has_next(list_t *list) {
    return list->curr_node != NULL;
}

bool list_iter_next(list_t *list, void *data) {
    if (list->curr_node == NULL) {
        return false;
    }

    memcpy(data, list->curr_node->data, list->elem_size);
    list->curr_node = list->curr_node->next;

    return true;
}

void list_iter_stop_safe(list_t *list) {
    pthread_mutex_unlock(&(list->mutex));
}
