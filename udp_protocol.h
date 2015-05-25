#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net.h"

#define MAGIC_CONNECTION_ID 0x41727101980
#define CONNECTION_ACTION   0
#define ANNOUNCE_ACTION     1

#define ANNOUNCE_EVENT_NONE      0
#define ANNOUNCE_EVENT_COMPLETED 1
#define ANNOUNCE_EVENT_STARTED   2
#define ANNOUNCE_EVENT_STOPPED   3

#define ANNOUNCE_NUM_WANT_DEFAULT -1
#define ANNOUNCE_NO_EXTENSIONS    0

typedef struct ConnectRequest {
    int64_t connection_id;
    int32_t action;
    int32_t transaction_id;
} ConnectRequest;

ssize_t send_connect_request(ConnectRequest *req,
    int sock, struct sockaddr *addr);

typedef struct ConnectResponse {
    int32_t action;
    int32_t transaction_id;
    int64_t connection_id;
} ConnectResponse;

ssize_t receive_connect_response(ConnectResponse* res, int sock);

typedef struct AnnounceRequest {
    int64_t  connection_id;
    int32_t  action;
    int32_t  transaction_id;
    int8_t   info_hash[20];
    int8_t   peer_id[20];
    int64_t  downloaded;
    int64_t  left;
    int64_t  uploaded;
    int32_t  event;
    uint32_t ip;
    uint32_t key;
    int32_t  num_want;
    uint16_t port;
    uint16_t extensions;
} AnnounceRequest;

ssize_t send_announce_request(AnnounceRequest *req,
    int sock, struct sockaddr *addr);

typedef struct PeerInfo {
    int32_t ip;
    uint16_t port;
    struct PeerInfo *next;
} PeerInfo;

typedef struct AnnounceResponse {
    int32_t action;
    int32_t transaction_id;
    int32_t interval;
    int32_t leechers;
    int32_t seeders;
    int32_t peer_count;
    PeerInfo *peers;
} AnnounceResponse;

ssize_t receive_announce_response(AnnounceResponse *res, int sock);
