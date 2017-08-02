#pragma once

#include <stdlib.h>

typedef struct peer {
    int ip_address;
    int port;

    char id[20];
    int bitfield_length;
    uint8_t *bitfield;
} peer_t;