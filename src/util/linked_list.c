#include "linked_list.h"

void linked_list_new(LinkedList *list, int elem_size) {
    list->head      = 0;
    list->tail      = 0;
    list->curr_node = 0;
    list->elem_size = elem_size;
    list->len       = 0;
}

int linked_list_len(LinkedList *list) {
    return list->len;
}

bool linked_list_empty(LinkedList *list) {
    return list->len == 0;
}

void linked_list_append(LinkedList *list, void *data) {
    Node *new_node = malloc(sizeof(Node));
    new_node->data = malloc(sizeof(list->elem_size));
    memcpy(new_node->data, data, list->elem_size);

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
}

bool linked_list_truncate(LinkedList *list) {
    if (list->len < 1) {
        return false;
    }

    Node *del_node = list->tail;
    if (list->len == 1) {
        list->head = 0;
        list->tail = 0;
    } else {
        list->tail = del_node->prev;
        del_node->prev->next = 0;
    }

    free(del_node);
    list->len--;
    return true;
}

bool linked_list_get(LinkedList *list, int idx, void *data) {
    if (idx < 0 || idx >= list->len) {
        return false;
    }

    int mid = list->len / 2;
    Node *node;

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

    return true;
}

void iter_start(LinkedList *list) {
    list->curr_node = list->head;
}

bool iter_has_next(LinkedList *list) {
    return list->curr_node != 0;
}

bool iter_next(LinkedList *list, void *data) {
    if (list->curr_node == 0) {
        return false;
    }

    memcpy(data, list->curr_node->data, list->elem_size);
    list->curr_node = list->curr_node->next;

    return true;
}
