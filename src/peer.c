#include "peer.h"

#include <string.h>
#include <stdlib.h>

void peer_update_bitfield(peer_t *peer, uint8_t *new_bitfield, uint32_t new_len) {
    if (peer->bitfield != NULL) {
        free(peer->bitfield);
    }

    peer->bitfield = malloc(new_len);
    memcpy(peer->bitfield, new_bitfield, new_len);
    peer->bitfield_length = new_len;
}

void peer_update_bitfield_index(peer_t *peer, uint32_t idx, bool newval) {
    uint32_t array_idx = idx / 8;
    if (array_idx >= peer->bitfield_length)
        return;

    // position within byte
    uint32_t byte_pos = 7 - (idx & 0x7);
    uint8_t mask = 1 << byte_pos;
    if (newval) {
        peer->bitfield[array_idx] |= mask;
    } else {
        peer->bitfield[array_idx] &= ~mask;
    }
}
