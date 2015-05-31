#include "peer_protocol.h"

#include <stdio.h>

ssize_t send_handshake_request(PeerHandshake *req,
    int sock, struct sockaddr *addr) {

    int  bufflen = 49 + req->pstrlen;
    char buffer[bufflen];

    // protocol string length
    buffer[0] = req->pstrlen;
    // protocol string
    memcpy(buffer + 1, req->pstr, req->pstrlen);
    // 8 reserved bytes
    *((int64_t*) (buffer + 1 + req->pstrlen)) = htonll(req->reserved);
    // info hash
    memcpy(buffer + 9 + req->pstrlen, req->info_hash, 20);
    // peer id
    memcpy(buffer + 29 + req->pstrlen, req->peer_id, 20);

    return sendto(sock, buffer, bufflen, 0, addr, sizeof(*addr));
}

ssize_t prompt_peer(int sock, struct sockaddr *addr) {
    char buf[17];
    // length prefix
    *((int32_t*) buf) = 13;

    buf[4] = 6; // id
    *((int32_t*) (buf + 5))  = 0;
    *((int32_t*) (buf + 9))  = 0;
    *((int32_t*) (buf + 13)) = 64;
    

    return sendto(sock, buf, 17, 0, addr, sizeof(*addr));
}

ssize_t receive_handshake_response(PeerHandshake *res,
    int sock, uint8_t pstrlen) {

    int  bufflen = pstrlen + 8 + 20 + 20; 
    char buffer[bufflen];

    ssize_t size = recv(sock, buffer, bufflen, 0);
    if (size != bufflen) {
        return size;
    }

    res->pstrlen = pstrlen;
    res->pstr    = malloc(pstrlen + 1);
    memcpy(res->pstr, buffer + 1, pstrlen);

    res->reserved = ntohll(*((uint64_t*) (buffer + 1 + pstrlen)));

    memcpy(res->info_hash, buffer + pstrlen + 8, 20);
    memcpy(res->peer_id, buffer + pstrlen + 28, 20);

    return size;
}

ssize_t receive_handshake_response_full(PeerHandshake *res, int sock) {
    uint8_t pstrlen;

    ssize_t size = recv(sock, &pstrlen, 1, 0);
    if (size < 1 || pstrlen != PROTOCOL_STR_LEN) {
        return -1;
    }

    return receive_handshake_response(res, sock, pstrlen);
}
