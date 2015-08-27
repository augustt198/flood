#include <stdint.h>
#include <sys/socket.h>

#define MAGIC_CONNECTION_ID 0x41727101980
#define CONNECTION_ACTION   0
#define ANNOUNCE_ACTION     1

#define ANNOUNCE_EVENT_NONE      0
#define ANNOUNCE_EVENT_COMPLETED 1
#define ANNOUNCE_EVENT_STARTED   2
#define ANNOUNCE_EVENT_STOPPED   3

#define ANNOUNCE_NUM_WANT_DEFAULT -1
#define ANNOUNCE_NO_EXTENSIONS    0

typedef struct udpt_connect_req {
    int64_t connection_id;
    int32_t action;
    int32_t transaction_id;
} udpt_connect_req;

ssize_t send_connect_request(udpt_connect_req *req,
    int sock, struct sockaddr *addr);

typedef struct udpt_connect_resp {
    int32_t action;
    int32_t transaction_id;
    int64_t connection_id;
} udpt_connect_resp;

ssize_t receive_connect_response(udpt_connect_resp* res, int sock);

typedef struct udpt_announce_req {
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
} udpt_announce_req;

ssize_t send_announce_request(udpt_announce_req *req,
    int sock, struct sockaddr *addr);

typedef struct udpt_peer {
    int32_t ip;
    uint16_t port;
    struct udpt_peer *next;
} udpt_peer;

typedef struct udpt_announce_resp {
    int32_t action;
    int32_t transaction_id;
    int32_t interval;
    int32_t leechers;
    int32_t seeders;
    int32_t peer_count;
    udpt_peer *peers;
} udpt_announce_resp;

ssize_t receive_announce_response(udpt_announce_resp *res, int sock);
