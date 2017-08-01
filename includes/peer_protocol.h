#pragma once

#include <stdint.h>
#include <sys/socket.h>

#define PEER_PROTOCOL_LEN 19

typedef struct peer_handshake {
    uint8_t pstrlen;
    char *pstr;
    uint64_t reserved;
    char info_hash[20];
    char peer_id[20];
} peer_handshake;

enum peer_message_type {
    PEER_MSG_KEEPALIVE,
    PEER_MSG_CHOKE,
    PEER_MSG_UNCHOKE,
    PEER_MSG_INTERESTED,
    PEER_MSG_NOTINTERESTED,
    PEER_MSG_HAVE,
    PEER_MSG_BITFIELD,
    PEER_MSG_REQUEST,
    PEER_MSG_PIECE,
    PEER_MSG_CANCEL,
    PEER_MSG_UNKNOWN
};

typedef struct peer_message {
    enum peer_message_type type;
    union {
        // when PEER_MSG_HAVE
        struct {
            uint32_t piece_index;
        } msg_have;
        // when PEER_MSG_BITFIELD
        struct {
            uint32_t bitfield_length; // not sent over network!
            uint8_t *bitfield;
        } msg_bitfield;
        // when PEER_MSG_REQUEST (or PEER_MSG_CANCEL)
        struct {
            uint32_t piece_index;
            uint32_t begin;
            uint32_t length;
        } msg_request;
        // when PEER_MSG_PIECE
        struct {
            uint32_t piece_index;
            uint32_t begin;
            uint32_t length; // not sent over network!
            uint8_t *data;
        } msg_piece;
    };
} peer_message;

ssize_t send_handshake_request(peer_handshake *req, int sock, struct sockaddr *addr);

ssize_t receive_handshake_request(peer_handshake *res, int sock, struct sockaddr *addr, uint8_t pstrlen);

ssize_t send_peer_message(peer_message *msg, int sock, struct sockaddr *addr);
ssize_t receive_peer_message(peer_message *msg, int sock, struct sockaddr *addr);