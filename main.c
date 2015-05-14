#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "uri_util.h"
#include "torrent.h"
#include "udp_protocol.h"
#include "time.h"

void die(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void handle_magnet(char *magnet);

int main(int argc, char **argv) {
    if (argc < 3) {
        die("Expected at least two arguments\n");
    }

    if (strcmp(argv[1], "magnet") == 0) {
        handle_magnet(argv[2]);
        return 0;
    } else if (strcmp(argv[1], "torrent") == 0) {
        die("Not supported yet\n");
    } else {
        fprintf(stderr, "Unknown option: %s\n", argv[1]);
        exit(1);
    }
}

void handle_magnet(char *magnet) {
    Torrent t;
    magnet2torrent(&t, magnet);
    
    if (t.tracker_count < 1)
        die("No trackers found\n");

    char *tracker = t.trackers[0];
    UriUriA tracker_uri;
    parse_uri(&tracker_uri, tracker);
    
    struct addrinfo *addr;
    getaddrinfo(uri_host(&tracker_uri), NULL, NULL, &addr);
    struct sockaddr_in *sock_addr = (struct sockaddr_in*) addr->ai_addr;

    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port   = htons(uri_port(&tracker_uri));
    
    ConnectRequest c_req;
    c_req.connection_id  = MAGIC_CONNECTION_ID;
    c_req.action         = CONNECTION_ACTION;
    srand(time(0));
    c_req.transaction_id = rand();

    printf("request transaction id: %d\n", c_req.transaction_id);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    send_connect_request(&c_req, sock, (struct sockaddr*) sock_addr);

    ConnectResponse c_resp;
    receive_connect_response(&c_resp, sock);
    printf("response transaction id: %d\n", c_resp.transaction_id);
    printf("connection id: %lld\n", c_resp.connection_id);

    AnnounceRequest a_req;
    a_req.connection_id  = c_resp.connection_id;
    a_req.action         = ANNOUNCE_ACTION;
    a_req.transaction_id = rand();

    memcpy(&a_req.info_hash, t.info_hash, 20);
    memcpy(&a_req.peer_id, "CUSTOM_CLIENT_123456", 20);

    a_req.downloaded = 0;
    a_req.left       = 0;
    a_req.uploaded   = 0;
    a_req.event      = ANNOUNCE_EVENT_NONE;
    a_req.ip         = 0;
    a_req.num_want   = ANNOUNCE_NUM_WANT_DEFAULT;
    a_req.port       = 1337;
    a_req.extensions = ANNOUNCE_NO_EXTENSIONS;

    send_announce_request(&a_req, sock, (struct sockaddr*) sock_addr);

    AnnounceResponse a_resp;
    receive_announce_response(&a_resp, sock);
    
    printf("Peers (%d):\n", a_resp.peer_count);
    PeerInfo *peer = a_resp.peers;
    while (peer) {
        uint32_t ip = peer->ip;
        printf(
            "%d.%d.%d.%d",
            ip >> 24, (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF, ip & 0xFF
        );
        printf(":%d\n", peer->port);

        peer = peer->next;
    }
}
