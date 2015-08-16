#include "udp_protocol.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int hexdig2dec(char dig) {
    if (dig >= '0' && dig <= '9') {
        return dig - '0';
    } else if (dig >= 'a' && dig <= 'f') {
        return dig - 'a' + 10;
    } else if (dig >= 'A' && dig <= 'F') {
        return dig - 'A' + 10;
    } else {
        return -1;
    }
}

char *hex2bin(char *hex, int len) {
    char *buf = malloc(len / 2);
    for (int i = 0; i < len; i += 2) {
        int upper = hexdig2dec(hex[i]);
        int lower = hexdig2dec(hex[i + 1]);
        char byte = (upper << 4) | lower;
        buf[i / 2] = byte;
    }

    return buf;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: [host] [port] [infohash]\n");
        exit(1);
    }

    char *host     = argv[1];
    char *portstr  = argv[2];
    char *infohash = argv[3];

    int port = atoi(portstr);

    struct addrinfo *ai;
    if (getaddrinfo(host, NULL, NULL, &ai) != 0) {
        perror(NULL);
        exit(1);
    }

    struct sockaddr *addr       = ai->ai_addr;
    struct sockaddr_in *addr_in = (struct sockaddr_in*) addr;

    addr_in->sin_port = htons(port);

    int transaction_id = rand();
    udpt_connect_req req = {
        MAGIC_CONNECTION_ID, CONNECTION_ACTION, transaction_id
    };

    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    
    ssize_t sentlen = send_connect_request(&req, sockfd, addr);   
    if (sentlen < 0) {
        fprintf(stderr, "Could not send connect request\n");
        exit(1);
    }

    udpt_connect_resp res;
    memset(&res, 0, sizeof(res));

    ssize_t recvlen = receive_connect_response(&res, sockfd);
    if (recvlen < 0) {
        fprintf(stderr, "Could not receive connect response\n");
        exit(1);
    }

    if (transaction_id != res.transaction_id) {
        fprintf(stderr, "Transaction IDs don't match\n");
        exit(1);
    }
    
    int64_t connection_id = res.connection_id;
    transaction_id = rand();

    udpt_announce_req a_req;
    memset(&a_req, 0, sizeof(udpt_announce_req));

    a_req.connection_id  = connection_id;
    a_req.action         = ANNOUNCE_ACTION;
    a_req.transaction_id = transaction_id;
    
    memcpy(a_req.info_hash, hex2bin(infohash, 40), 20);
    memcpy(a_req.peer_id, "AUGUST_CLIENT_123456", 20);

    a_req.downloaded = 0;
    a_req.left       = 0;
    a_req.uploaded   = 0;
    a_req.event      = ANNOUNCE_EVENT_STARTED;
    a_req.ip         = 0;
    a_req.key        = rand();
    a_req.num_want   = 50;

    sentlen = send_announce_request(&a_req, sockfd, addr);

    udpt_announce_resp a_res;
    memset(&a_res, 0, sizeof(a_res));

    recvlen = receive_announce_response(&a_res, sockfd);

    printf(
        "Seeders: %d, leechers: %d\n",
        a_res.seeders,
        a_res.leechers
    );
    
    printf("Peers (%d):\n", a_res.peer_count);

    udpt_peer *peer = a_res.peers;
    while (peer != NULL) {
        int ip = peer->ip;
        printf(
            "%d.%d.%d.%d:%d\n",
            (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF,
            peer->port
        );
        peer = peer->next;
    }
}
