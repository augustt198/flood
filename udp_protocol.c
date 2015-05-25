#include "udp_protocol.h"
#include <stdio.h>

#define CONNECT_REQUEST_SIZE  16
#define CONNECT_RESPONSE_SIZE 16
#define ANNOUNCE_REQUEST_SIZE 100
#define ANNOUNCE_RESPONSE_MIN_SIZE 20

ssize_t send_connect_request(ConnectRequest *req,
    int sock, struct sockaddr *addr) {

    char buffer[CONNECT_REQUEST_SIZE];
    *((long long *) (buffer)) = htonll(req->connection_id);
    *((int*) (buffer + 8))    = htonl(req->action);
    *((int*) (buffer + 12))   = htonl(req->transaction_id);

    return sendto(sock, buffer, CONNECT_REQUEST_SIZE, 0, addr, sizeof(*addr));
}

ssize_t receive_connect_response(ConnectResponse* res, int sock) {

    char buffer[CONNECT_RESPONSE_SIZE];
    ssize_t size = recvfrom(sock, buffer, CONNECT_RESPONSE_SIZE, 0, NULL, NULL);
    if (size < CONNECT_RESPONSE_SIZE)
        return size;

    res->action         = ntohl(*((int*) buffer));
    res->transaction_id = ntohl(*((int*) (buffer + 4)));
    res->connection_id  = ntohll(*(long long *) (buffer + 8));

    return size;
}

ssize_t send_announce_request(AnnounceRequest *req,
    int sock, struct sockaddr *addr) {

    char buffer[ANNOUNCE_REQUEST_SIZE];
    memset(buffer, 0, ANNOUNCE_REQUEST_SIZE);

    *((long long *) buffer) = htonll(req->connection_id);
    *((int*) (buffer + 8))  = htonl(req->action);
    *((int*) (buffer + 12)) = htonl(req->transaction_id);
    memcpy(buffer + 16, req->info_hash, 20);
    memcpy(buffer + 36, req->peer_id, 20);
    *((long long *) (buffer + 56)) = htonll(req->downloaded);
    *((long long *) (buffer + 64)) = htonll(req->left);
    *((long long *) (buffer + 72)) = htonll(req->uploaded);
    *((int*) (buffer + 80))   = htonl(req->event);
    *((int*) (buffer + 84))   = htonl(req->ip);
    *((int*) (buffer + 88))   = htonl(req->key);
    *((int*) (buffer + 92))   = htonl(req->num_want);
    *((short*) (buffer + 96)) = htons(req->port);
    *((short*) (buffer + 98)) = htons(req->extensions);

    return sendto(sock, buffer, ANNOUNCE_REQUEST_SIZE, 0, addr, sizeof(*addr));
}

ssize_t receive_announce_response(AnnounceResponse *res, int sock) {
    char buffer[1024];
    ssize_t size = recvfrom(sock, buffer, 1024, 0, NULL, NULL);
    if (size < ANNOUNCE_RESPONSE_MIN_SIZE)
        return size;

    res->action         = ntohl(*((int*) buffer));
    res->transaction_id = ntohl(*((int*) (buffer + 4)));
    res->interval       = ntohl(*((int*) (buffer + 8)));
    res->leechers       = ntohl(*((int*) (buffer + 12)));
    res->seeders        = ntohl(*((int*) (buffer + 16)));

    res->peer_count = (size - ANNOUNCE_RESPONSE_MIN_SIZE) / 6;

    PeerInfo *peer = 0;
    for (int i = 20; i < size; i += 6) {
        PeerInfo *next_peer = malloc(sizeof(PeerInfo));
        if (res->peers == NULL) {
            res->peers = next_peer;
        } else {
            peer->next = next_peer;
        }

        next_peer->ip   = ntohl(*((int*) (buffer + i)));
        next_peer->port = ntohs(*((int*) (buffer + i + 4)));

        peer = next_peer;
    }

    return size;
}
