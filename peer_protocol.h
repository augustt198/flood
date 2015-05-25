#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net.h"

typedef struct PeerHandshake {
    int32_t pstrlen;
    char*   pstr;
    int64_t reserved;
    char    info_hash[20];
    char    peer_id[20];
} PeerHandshake;

ssize_t send_handshake_request(PeerHandshake *req,
    int sock, struct sockaddr *addr);

ssize_t receive_handshake_response(PeerHandshake *res, int sock);

ssize_t prompt_peer(int sock, struct sockaddr *addr);
