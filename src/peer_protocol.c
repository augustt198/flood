#include "peer_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <util.h>

#include "net.h"

ssize_t send_peer_handshake(peer_handshake *req, int sock, struct sockaddr *addr) {
    int buflen = 49 + req->pstrlen;
    char buffer[buflen];

    buffer[0] = req->pstrlen;
    memcpy(buffer + 1, req->pstr, req->pstrlen);
    *((int64_t*) (buffer+ 1 + req->pstrlen)) = htonll(req->reserved);
    memcpy(buffer + 9 + req->pstrlen, req->info_hash, 20);
    memcpy(buffer + 29 + req->pstrlen, req->peer_id, 20);

    return sendto(sock, buffer, buflen, 0, addr, sizeof(*addr));
}

ssize_t receive_peer_handshake(peer_handshake *res, int sock, struct sockaddr *addr) {
    uint8_t pstrlen = 19;
    int buflen = 1 + pstrlen + 8 + 20 + 20;
    char buffer[buflen];
    memset(buffer, 0, buflen);

    ssize_t size = recv(sock, buffer, buflen, 0);

    res->pstrlen = buffer[0];
    res->pstr    = calloc(1, pstrlen+1); // additional byte for \0
    memcpy(res->pstr, buffer+1, pstrlen);
    res->reserved = ntohll(*((uint64_t*) (buffer+pstrlen)));

    memcpy(res->info_hash, buffer + 1 + pstrlen + 8, 20);
    memcpy(res->peer_id, buffer + 1 + pstrlen + 8 + 20, 20);

    return size;
}

ssize_t send_peer_message(peer_message *msg, int sock, struct sockaddr *addr) {
    uint32_t msg_length;
    uint8_t msg_id;

    if (msg->type == PEER_MSG_KEEPALIVE) {
       msg_length = 0;
    } else if (msg->type == PEER_MSG_CHOKE) {
        msg_length = 1;
        msg_id = 0;
    } else if (msg->type == PEER_MSG_UNCHOKE) {
        msg_length = 1;
        msg_id = 1;
    } else if (msg->type == PEER_MSG_INTERESTED) {
        msg_length = 1;
        msg_id = 2;
    } else if (msg->type == PEER_MSG_NOTINTERESTED) {
        msg_length = 1;
        msg_id = 3;
    } else if (msg->type == PEER_MSG_HAVE) {
        msg_length = 5;
        msg_id = 4;
    } else if (msg->type == PEER_MSG_BITFIELD) {
        msg_length = 1 + msg->msg_bitfield.bitfield_length;
        msg_id = 5;
    } else if (msg->type == PEER_MSG_REQUEST) {
        msg_length = 13;
        msg_id = 6;
    } else if (msg->type == PEER_MSG_PIECE) {
        msg_length = 9 + msg->msg_piece.length;
        msg_id = 7;
    } else if (msg->type == PEER_MSG_CANCEL) {
        msg_length = 13;
        msg_id = 8;
    } else {
        msg_length = -1;
    }

    ssize_t sb = 0; // sent bytes

    uint32_t network_msg_length = htonl(msg_length);
    sb += sendto(sock, &network_msg_length, 4, 0, addr, sizeof(*addr));
    if (msg_length == 0)
        return sb;

    sb += send(sock, &msg_id, 1, 0);
    if (msg->type == PEER_MSG_HAVE) {
        uint32_t network_piece_index = htonl(msg->msg_request.piece_index);
        sb += send(sock, &network_piece_index, 4, 0);

    } else if (msg->type == PEER_MSG_BITFIELD) {
        sb += send(sock, msg->msg_bitfield.bitfield, msg->msg_bitfield.bitfield_length, 0);

    } else if (msg->type == PEER_MSG_REQUEST || msg->type == PEER_MSG_CANCEL) {
        uint32_t network_piece_index = htonl(msg->msg_request.piece_index);
        sb += send(sock, &network_piece_index, 4, 0);
        uint32_t network_begin = htonl(msg->msg_request.begin);
        sb += send(sock, &network_begin, 4, 0);
        uint32_t network_length = htonl(msg->msg_request.length);
        sb += send(sock, &network_length, 4, 0);

    } else if (msg->type == PEER_MSG_PIECE) {
        uint32_t network_piece_index = htonl(msg->msg_piece.piece_index);
        sb += send(sock, &network_piece_index, 4, 0);
        uint32_t network_begin = htonl(msg->msg_piece.begin);
        sb += send(sock, &network_begin, 4, 0);

        sb += send(sock, msg->msg_piece.data, msg->msg_piece.length, 0);
    }

    return sb;
}

ssize_t receive_peer_message(peer_message *msg, int sock, struct sockaddr *addr) {
    ssize_t rb = 0; // received bytes

    // read length prefix
    uint32_t network_length_prefix;
    rb += recvfrom(sock, &network_length_prefix, 4, 0, addr, NULL);
    uint32_t length_prefix = ntohl(network_length_prefix);

    // keep alive message
    if (length_prefix == 0) {
        msg->type = PEER_MSG_KEEPALIVE;
    }

    uint8_t msg_type;
    rb += recv(sock, &msg_type, 1, 0);

    if (msg_type == 0) {
        msg->type = PEER_MSG_CHOKE;
    } else if (msg_type == 1) {
        msg->type = PEER_MSG_UNCHOKE;
    } else if (msg_type == 2) {
        msg->type = PEER_MSG_INTERESTED;
    } else if (msg_type == 3) {
        msg->type = PEER_MSG_NOTINTERESTED;
    } else if (msg_type == 4) {
        msg->type = PEER_MSG_HAVE;
        uint32_t network_piece_index;
        recv(sock, &network_piece_index, 4, 0);
        msg->msg_have.piece_index = ntohl(network_piece_index);

    } else if (msg_type == 5) {
        msg->type = PEER_MSG_BITFIELD;
        uint32_t bflen = length_prefix - 1;
        msg->msg_bitfield.bitfield_length = bflen;
        msg->msg_bitfield.bitfield = malloc(bflen);
        recv(sock, msg->msg_bitfield.bitfield, bflen, 0);

    } else if (msg_type == 6 || msg_type == 8) {
        msg->type = msg_type == 6 ? PEER_MSG_REQUEST : PEER_MSG_CANCEL;

        uint32_t network_piece_index;
        recv(sock, &network_piece_index, 4, 0);
        msg->msg_request.piece_index = ntohl(network_piece_index);
        uint32_t network_begin;
        recv(sock, &network_begin, 4, 0);
        msg->msg_request.begin = ntohl(network_begin);
        uint32_t network_length;
        recv(sock, &network_length, 4, 0);
        msg->msg_request.length = ntohl(network_length);

    } else if (msg_type == 7) {
        msg->type = PEER_MSG_PIECE;

        uint32_t network_piece_index;
        recv(sock, &network_piece_index, 4, 0);
        msg->msg_piece.piece_index = ntohl(network_piece_index);
        uint32_t network_begin;
        recv(sock, &network_begin, 4, 0);
        msg->msg_piece.begin = ntohl(network_begin);

        uint32_t piece_length = length_prefix - 9;
        msg->msg_piece.length = piece_length;
        msg->msg_piece.data = malloc(piece_length);
        recv(sock, msg->msg_piece.data, piece_length, 0);
    } else {
        msg->type = PEER_MSG_UNKNOWN;
        printf("GOT UNKNOWN MESSAGE TYPE ID: %d / [len %u]\n", msg_type, length_prefix);
    }

    return rb;
}

void print_peer_message(peer_message *msg) {
    printf("Message: ");
    enum peer_message_type t = msg->type;
    if (t == PEER_MSG_KEEPALIVE)
        printf("keep alive");
    if (t == PEER_MSG_CHOKE)
        printf("choke");
    if (t == PEER_MSG_UNCHOKE)
        printf("unchoke");
    if (t == PEER_MSG_INTERESTED)
        printf("interested");
    if (t == PEER_MSG_NOTINTERESTED)
        printf("not interested");
    if (t == PEER_MSG_HAVE)
        printf("have");
    if (t == PEER_MSG_BITFIELD)
        printf("bitfield");
    if (t == PEER_MSG_REQUEST)
        printf("request");
    if (t == PEER_MSG_PIECE)
        printf("piece");
    if (t == PEER_MSG_CANCEL)
        printf("cancel");
    if (t == PEER_MSG_UNKNOWN) {
        printf("unknown");
    }
    printf("\n");
}
