#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct LinkedList {
    Node *head;
    Node *tail;
    Node *curr_node;
    int elem_size;
    int len;
} LinkedList;

// Initializes the linked list at `list`, with
// the size of each element set to `elem_size`.
void linked_list_new(LinkedList *list, int elem_size);

// Gets the length of the linked list.
int linked_list_len(LinkedList *list);

// Returns whether or not the linked list
// is empty (lenght == 0)
bool linked_list_empty(LinkedList *list);

// Appends `data` to the end of the list.
void linked_list_append(LinkedList *list, void *data);

// Removes the last element from list.
//
// Returns `true` if any elements were
// removed, `false` if not.
bool linked_list_truncate(LinkedList *list);

// Fetches the element at `idx` from `list`
// and places it into `data`.
//
// Returns `true` if `idx` was a valid index
// into `list`, `false` if not.
bool linked_list_get(LinkedList *list, int idx, void *data);

void iter_start(LinkedList *list);

bool iter_has_next(LinkedList *list);

bool iter_next(LinkedList *list, void *data);
