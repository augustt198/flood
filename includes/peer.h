#pragma once

#include <stdint.h>
#include <stdbool.h>

enum peer_status {
    PEER_STATUS_NEW,
    PEER_STATUS_IN_USE,
    PEER_STATUS_BAD
};

typedef struct peer {
    int ip_address;
    int port;

    enum peer_status status;

    // fields obtained from handshake
    char id[20];

    // fields obtained through messages
    int bitfield_length;
    uint8_t *bitfield;
    bool choking;       // whether peer is choking us
    bool interested;    // whether peer is interested in us

    // which tracker it was discovered through
    char *origin_tracker;
} peer_t;

void peer_update_bitfield(peer_t *peer, uint8_t *new_bitfield, uint32_t new_len);
void peer_update_bitfield_index(peer_t *peer, uint32_t idx, bool newval);
