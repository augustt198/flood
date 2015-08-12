#pragma once

#include <stdbool.h>
#include <pthread.h>

typedef struct ListNode {
    void *data;
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

typedef struct List {
    ListNode *head;
    ListNode *tail;
    ListNode *curr_node;
    int elem_size;
    int len;
    pthread_mutex_t mutex;
} List;


// Initializes the linked list at `list`, with
// the size of each element set to `elem_size`.
void list_new(List *list, int elem_size);

// Gets the length of the linked list.
int list_len(List *list);

// Returns whether or not the linked list
// is empty (lenght == 0)
bool list_empty(List *list);

// Appends `data` to the end of the list.
void list_append(List *list, void *data);

// Prepends `data` to the start of the list.
void list_prepend(List *list, void *data);

// Inserts `data` at index `idx`.
bool list_insert(List *list, int idx, void *data);

// Removes the last element from list.
//
// Returns `true` if any elements were
// removed, `false` if not.
bool list_truncate(List *list);

void list_each(List *list, int (*fn)(int, void *));

// Fetches the element at `idx` from `list`
// and places it into `data`.
//
// Returns `true` if `idx` was a valid index
// into `list`, `false` if not.
bool list_get(List *list, int idx, void *data);

void list_iter_start(List *list);

bool list_iter_has_next(List *list);

bool list_iter_next(List *list, void *data);
