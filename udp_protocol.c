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
    char buffer[512];
    ssize_t size = recvfrom(sock, buffer, 512, 0, NULL, NULL);
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

int main() {
    struct sockaddr_in addr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    // open.demonii.com
    addr.sin_addr.s_addr = inet_addr("185.90.63.2");
    addr.sin_port = htons(1337);

    ConnectRequest creq = {0x41727101980, 0, 1337};
    send_connect_request(&creq, sockfd, (struct sockaddr*) &addr);

    ConnectResponse cresp;
    int size = receive_connect_response(&cresp, sockfd);

    char info_hash[20] = {
        6, 22, 192, 245, 72, 27, 200, 80, 102, 144, 226,
        18, 249, 204, 55, 223, 209, 71, 26, 51
    };
    char peer_id[20] = "00000000000000000000";
    int ip = 0;
    AnnounceRequest areq;
    bzero(&areq, sizeof(AnnounceRequest));
    areq.connection_id = cresp.connection_id;
    areq.action = 1;
    areq.transaction_id = 8080;
    memcpy(&(areq.info_hash), &info_hash, 20);
    memcpy(&(areq.peer_id), &peer_id, 20);
    areq.downloaded = 0;
    areq.left = 0;
    areq.uploaded = 0;
    areq.event = 0;
    areq.ip = ip;
    areq.key = 666;
    areq.num_want = 10;
    areq.port = 1337;
    areq.extensions = 0;

    size = send_announce_request(&areq, sockfd, (struct sockaddr*) &addr);

    AnnounceResponse aresp;
    receive_announce_response(&aresp, sockfd);

    printf("Peers:\n");

    PeerInfo *peer = aresp.peers;
    while (peer != NULL) {
        uint32_t ip = peer->ip;
        printf(
            "%d.%d.%d.%d:%d\n",
            ip >> 24, (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF, ip & 0xFF,
            peer->port
        );
        peer = peer->next;
    }
}
