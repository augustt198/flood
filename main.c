#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <errno.h>

#include "uri_util.h"
#include "torrent.h"
#include "udp_protocol.h"
#include "peer_protocol.h"


void die(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void init_sock_opts(int sock) {
    struct timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    setsockopt(
        sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &tv, sizeof(struct timeval)
    );
}

void handle_magnet(char *magnet);

char *fmt_ver(char *ver);

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

struct ThreadInfo {
    PeerInfo *peer;
    char *info_hash;
};

void *handle_peer(void *peer);

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
    
    ConnectRequest c_req = {};
    c_req.connection_id  = MAGIC_CONNECTION_ID;
    c_req.action         = CONNECTION_ACTION;
    srand(time(0));
    c_req.transaction_id = rand();

    printf("request transaction id: %d\n", c_req.transaction_id);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    init_sock_opts(sock);
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

    AnnounceResponse a_resp = {};
    receive_announce_response(&a_resp, sock);
    
    printf("Peers (%d):\n", a_resp.peer_count);
    pthread_t threads[a_resp.peer_count];
    PeerInfo *peer = a_resp.peers;
    for (int i = 0; peer; i++) {
        struct ThreadInfo info = {
            peer, t.info_hash
        };
        pthread_create(&threads[i], NULL, handle_peer, &info);

        peer = peer->next;
    }
    for (int i = 0; i < a_resp.peer_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

static const char *client_id_table[][2] = {
    {"UT", "uTorrent"},
    {"AZ", "Azureus"},
    {"TR", "Transmission"},
    {"BC", "BitComet"},
    {"LT", "libtorrent"},
    {"BT", "BitTorrent"}
};

const char *client_name_lookup(char *id) {
    int len = sizeof(client_id_table) / sizeof(client_id_table[0]);
    for (int i = 0; i < len; i++) {
        if (id[0] == client_id_table[i][0][0] &&
            id[1] == client_id_table[i][0][1])
            return client_id_table[i][1];
    }
    return "Unknown";
}

char *fmt_ver(char *ver) {
    char *fmt = malloc(8);
    memset(fmt, 0, 8);

    
    for (int i = 0; i < 4; i++) {
        outer:
        if (ver[i] != '0' || i == 0) {
            fmt[i * 2] = ver[i];
            if (i > 0)
                fmt[i * 2 - 1] = '.';
        } else {
            for (int j = i + 1; j < 4; j++) {
                if (ver[j] != '0') {
                    fmt[i * 2] = '0';
                    if (i > 0)
                        fmt[i * 2 - 1] = '.';
                    i++;
                    goto outer;
                }
            }
            break;
        }
    }

    return fmt;
}

void *handle_peer(void *msg) {
    struct ThreadInfo *info = (struct ThreadInfo*) msg;
    PeerInfo *peer   = info->peer;
    char *info_hash  = info->info_hash;
    uint32_t ip = peer->ip;

    struct sockaddr_in peeraddr;
    peeraddr.sin_family         = AF_INET;
    peeraddr.sin_addr.s_addr    = htonl(ip);
    peeraddr.sin_port           = htons(peer->port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    init_sock_opts(sock);
    
    int status = connect(sock, (struct sockaddr*) &peeraddr, sizeof(struct sockaddr_in));
    if (status != 0) {
        printf("error: %s\n", strerror(errno));
        return NULL;
    }

    PeerHandshake h_req;
    h_req.pstrlen = 19;
    h_req.pstr = "BitTorrent protocol";
    h_req.reserved = 1 << 20;
    memcpy(&h_req.info_hash, info_hash, 20);
    memcpy(&h_req.peer_id, "CUSTOM_CLIENT_123456", 20);

    send_handshake_request(&h_req, sock, (struct sockaddr*) &peeraddr);

    PeerHandshake h_res;
    ssize_t r_size = receive_handshake_response(&h_res, sock);
    if (r_size < 0) {
        printf("cannot recv: %s\n", strerror(errno));
        return NULL;
    }

    printf(
            "%d.%d.%d.%d:%d client: %s %s\n",
            ip >> 24, (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF, ip & 0xFF,
            peer->port,
            client_name_lookup(h_res.peer_id + 1),
            fmt_ver(h_res.peer_id + 3)
    );

    return NULL;
}
