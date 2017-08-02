#pragma once

#include <stdlib.h>

enum peer_status {
    PEER_STATUS_NEW,
    PEER_STATUS_IN_USE,
    PEER_STATUS_BAD
};

typedef struct peer {
    int ip_address;
    int port;

    enum peer_status status;

    char id[20];
    int bitfield_length;
    uint8_t *bitfield;

    // which tracker it was discovered through
    char *origin_tracker;
} peer_t;
