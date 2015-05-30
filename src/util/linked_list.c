#include "linked_list.h"

void linked_list_new(LinkedList *list, int elem_size) {
    list->head      = 0;
    list->tail      = 0;
    list->curr_node = 0;
    list->elem_size = elem_size;
    list->len       = 0;
    pthread_mutex_init(&(list->mutex), NULL);
}

int linked_list_len(LinkedList *list) {
    return list->len;
}

bool linked_list_empty(LinkedList *list) {
    return list->len == 0;
}

Node *create_node(LinkedList *list, void *data) {
    Node *node = malloc(sizeof(Node));
    node->data = malloc(sizeof(list->elem_size));
    memcpy(node->data, data, list->elem_size);

    return node;
}

void linked_list_append(LinkedList *list, void *data) {
    pthread_mutex_lock(&(list->mutex));

    Node *new_node = create_node(list, data);

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

void linked_list_prepend(LinkedList *list, void *data) {
    pthread_mutex_lock(&(list->mutex));

    Node *new_node = create_node(list, data);

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


bool linked_list_insert(LinkedList *list, int idx, void *data) {
    if (idx < 0 || idx > list->len)
        return false;

    if (idx == list->len) {
        linked_list_append(list, data);
        return true;
    } else if (idx == 0) {
        linked_list_prepend(list, data);
        return true;
    }
    pthread_mutex_lock(&(list->mutex));


    Node *new_node = create_node(list, data);
    Node *node;
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

bool linked_list_truncate(LinkedList *list) {
    pthread_mutex_lock(&(list->mutex));
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

    pthread_mutex_unlock(&(list->mutex));
    return true;
}

bool linked_list_get(LinkedList *list, int idx, void *data) {
    if (idx < 0 || idx >= list->len) {
        return false;
    }
    pthread_mutex_lock(&(list->mutex));

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
    pthread_mutex_unlock(&(list->mutex));
    return true;
}

void linked_list_each(LinkedList *list, int (*fn)(int, void *)) {
    Node *node = list->head;
    for (int i = 0; node; i++) {
        if (fn(i, node->data) != 0)
            break;

        node = node->next;
    }
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
