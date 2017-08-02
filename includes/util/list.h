#pragma once

#include <stdbool.h>
#include <pthread.h>

typedef struct list_node {
    void *data;
    struct list_node *next;
    struct list_node *prev;
} list_node;

typedef struct list_t {
    list_node *head;
    list_node *tail;
    list_node *curr_node;
    int elem_size;
    int len;
    pthread_mutex_t mutex;
} list_t;


// Initializes the linked list at `list`, with
// the size of each element set to `elem_size`.
void list_new(list_t *list, int elem_size);

// Gets the length of the linked list.
int list_len(list_t *list);

// Returns whether or not the linked list
// is empty (lenght == 0)
bool list_empty(list_t *list);

// Appends `data` to the end of the list.
void list_append(list_t *list, void *data);

// Prepends `data` to the start of the list.
void list_prepend(list_t *list, void *data);

// Inserts `data` at index `idx`.
bool list_insert(list_t *list, int idx, void *data);

// Removes the last element from list.
//
// Returns `true` if any elements were
// removed, `false` if not.
bool list_truncate(list_t *list);

void list_each(list_t *list, int (*fn)(int, void *));

// Fetches the element at `idx` from `list`
// and places it into `data`.
//
// Returns `true` if `idx` was a valid index
// into `list`, `false` if not.
bool list_get(list_t *list, int idx, void *data);

void list_iter_start(list_t *list);
void list_iter_start_safe(list_t *list);

bool list_iter_has_next(list_t *list);

bool list_iter_next(list_t *list, void *data);

void list_iter_stop_safe(list_t *list);