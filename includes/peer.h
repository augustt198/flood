#pragma once

#include <stdlib.h>

typedef struct peer {
    int ip_address;
    int port;

    char id[20];
    int bitfield_length;
    uint8_t *bitfield;

    // which tracker it was discovered through
    char *origin_tracker;
} peer_t;
