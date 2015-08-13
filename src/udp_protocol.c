#include "udp_protocol.h"

#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CONNECT_REQUEST_SIZE       16
#define CONNECT_RESPONSE_SIZE      16
#define ANNOUNCE_REQUEST_SIZE      100
#define ANNOUNCE_RESPONSE_MIN_SIZE 20

ssize_t send_connect_request(udpt_connect_req *req,
    int sock, struct sockaddr *addr) {

    char buffer[CONNECT_REQUEST_SIZE];
    *((uint64_t*) (buffer)) = htonll(req->connection_id);
    *((uint32_t*) (buffer + 8))    = htonl(req->action);
    *((uint32_t*) (buffer + 12))   = htonl(req->transaction_id);

    return sendto(sock, buffer, CONNECT_REQUEST_SIZE, 0, addr, sizeof(*addr));
}

ssize_t receive_connect_response(udpt_connect_resp* res, int sock) {
    char buffer[CONNECT_RESPONSE_SIZE];
    ssize_t size = recvfrom(sock, buffer, CONNECT_RESPONSE_SIZE, 0, NULL, NULL);
    if (size < CONNECT_RESPONSE_SIZE)
        return size;

    res->action         = ntohl(*((uint32_t*) buffer));
    res->transaction_id = ntohl(*((uint32_t*) (buffer + 4)));
    res->connection_id  = ntohll(*(uint64_t*) (buffer + 8));

    return size;
}

ssize_t send_announce_request(udpt_announce_req *req,
    int sock, struct sockaddr *addr) {

    char buffer[ANNOUNCE_REQUEST_SIZE];
    memset(buffer, 0, ANNOUNCE_REQUEST_SIZE);

    *((uint64_t*) (buffer + 0))  = htonll(req->connection_id);
    *((uint32_t*) (buffer + 8))  = htonl (req->action);
    *((uint32_t*) (buffer + 12)) = htonl (req->transaction_id);
    memcpy(buffer + 16, req->info_hash, 20);
    memcpy(buffer + 36, req->peer_id, 20);
    *((uint64_t*) (buffer + 56)) = htonll(req->downloaded);
    *((uint64_t*) (buffer + 64)) = htonll(req->left);
    *((uint64_t*) (buffer + 72)) = htonll(req->uploaded);
    *((uint32_t*) (buffer + 80)) = htonl (req->event);
    *((uint32_t*) (buffer + 84)) = htonl (req->ip);
    *((uint32_t*) (buffer + 88)) = htonl (req->key);
    *((uint32_t*) (buffer + 92)) = htonl (req->num_want);
    *((uint16_t*) (buffer + 96)) = htons (req->port);
    *((uint16_t*) (buffer + 98)) = htons (req->extensions);

    return sendto(sock, buffer, ANNOUNCE_REQUEST_SIZE, 0, addr, sizeof(*addr));
}

ssize_t receive_announce_response(udpt_announce_resp *res, int sock) {
    char buffer[1024];
    ssize_t size = recvfrom(sock, buffer, 1024, 0, NULL, NULL);
    if (size < ANNOUNCE_RESPONSE_MIN_SIZE)
        return size;

    res->action         = ntohl(*((uint32_t*) (buffer + 0)));
    res->transaction_id = ntohl(*((uint32_t*) (buffer + 4)));
    res->interval       = ntohl(*((uint32_t*) (buffer + 8)));
    res->leechers       = ntohl(*((uint32_t*) (buffer + 12)));
    res->seeders        = ntohl(*((uint32_t*) (buffer + 16)));

    res->peer_count = (size - ANNOUNCE_RESPONSE_MIN_SIZE) / 6;

    udpt_peer *peer = NULL;
    for (int i = 20; i < size; i += 6) {
        udpt_peer *next_peer = malloc(sizeof(udpt_peer));
        if (peer == NULL) {
            res->peers = next_peer;
        } else {
            peer->next = next_peer;
        }

        next_peer->ip   = ntohl(*((uint32_t*) (buffer + i + 0)));
        next_peer->port = ntohs(*((uint32_t*) (buffer + i + 4)));

        peer = next_peer;
    }

    return size;
}
