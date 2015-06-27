#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "uri_util.h"
#include "torrent.h"
#include "discover_ip.h"

#include "udp_protocol.h"
#include "peer_protocol.h"

typedef struct Client {
    char *peer_id;
    struct in_addr *ip;
    int  port;
} Client;

typedef struct Swarm {
    Client     *client;
    Torrent    *torrent;
    // peers associated with swarm,
    // sorted by IP address (ascending)
    LinkedList *peers;
    char       *peer_id;
    // mutex for updating peer list
    pthread_mutex_t peer_mutex;
} Swarm;

typedef struct Peer {
    uint32_t ip;
    uint16_t port;

    bool alive;
    LinkedList *messages;

    int sock; // socket
    struct sockaddr* addr;
} Peer;

void init_swarm(Swarm *swarm, Client *c, Torrent *t);
