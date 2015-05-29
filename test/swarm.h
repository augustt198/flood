#include <pthread.h>

#include "linked_list.h"

typedef struct Swarm {
    Torrent *torrent;
    LinkedList *peers;
} Swarm;

typedef struct Peer {
    LinkedList *responses;
} Peer;
