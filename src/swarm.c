#include "swarm.h"

struct pr_info {
    Swarm *swarm;
    Peer  *peer;
};

void *handle_peer(void *data) {
    struct pr_info *info = (struct pr_info*) data;
    Torrent *torrent     = info->swarm->torrent;
    Peer *peer           = info->peer;
    free(info);

    PeerHandshake handshake;
    ssize_t size = receive_handshake_response(&handshake, peer->sock, PROTOCOL_STR_LEN);
    if (size < 0) {
        return NULL;
    }

    if (memcmp(torrent->info_hash, handshake.info_hash, 20) == 0) {
        printf(">> client has correct info hash\n");
    }
    
    if ((handshake.reserved >> 44) & 1) {
        printf(">> client supports extension protocol\n");
    }

    return NULL;
}

void *try_peer(void *data) {
    struct pr_info *info = (struct pr_info*) data;
    Swarm *swarm = info->swarm;
    Peer *peer   = info->peer;

    struct sockaddr_in peeraddr;
    peeraddr.sin_family         = AF_INET;
    peeraddr.sin_addr.s_addr    = htonl(peer->ip);
    peeraddr.sin_port           = htons(peer->port);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    peer->sock = sock;
    int status = connect(
        sock, (struct sockaddr*) &peeraddr, sizeof(struct sockaddr_in)
    );
    if (status != 0) {
        peer->alive = false;
        free(info);
        return NULL;
    }

    PeerHandshake h_req;
    h_req.pstrlen = 19;
    h_req.pstr = "BitTorrent protocol";
    h_req.reserved = 1 << 20;
    memcpy(&h_req.info_hash, swarm->torrent->info_hash, 20);
    memcpy(&h_req.peer_id, "CUSTOM_CLIENT_123456", 20);

    send_handshake_request(&h_req, sock, (struct sockaddr*) &peeraddr);

    char plen;
    int size = recv(sock, &plen, 1, 0);
    if (size < 0) {
        free(info);
        return NULL;
    }
    if (plen == 19) {
        printf(">> found alive peer\n");
        pthread_t thread;
        pthread_create(&thread, NULL, handle_peer, info);
    } else {
        free(info);
    }

    return NULL;
}

// returns: whether or not the peer was added
bool add_peer(Swarm *s, uint32_t ip, uint16_t port) {
    pthread_mutex_lock(&(s->peer_mutex));

    // insert in sorted position (ascending)
    LinkedList *l = s->peers;
    iter_start(l);
    int idx = 0;
    while (iter_has_next(l)) {
        Peer *p;
        iter_next(l, &p);

        if (ip == p->ip) {
            pthread_mutex_unlock(&(s->peer_mutex));
            return false;
        }
        
        if (ip < p->ip)
            break;
        else
            idx++;
    }

    Peer *peer  = malloc(sizeof(Peer));
    peer->ip    = ip;
    peer->port  = port;
    peer->alive = true;

    peer->messages = malloc(sizeof(LinkedList));
    linked_list_new(peer->messages, 0);

    linked_list_insert(l, idx, &peer);

    if (idx % 5 == 0) {
        struct pr_info* info = malloc(sizeof(struct pr_info)); //{s, peer};
        info->swarm = s;
        info->peer  = peer;
        pthread_t thread;
        pthread_create(&thread, NULL, try_peer, info);
    }

    pthread_mutex_unlock(&(s->peer_mutex));
    return true;
}

struct tr_info {
    Swarm *swarm;
    char *tr_host;
    struct sockaddr_in *tr_addr;
};

void *tracker_routine(void *data) {
    struct tr_info *info = (struct tr_info*) data;
    Client *client       = info->swarm->client;
    Torrent *torrent     = info->swarm->torrent;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    ConnectRequest conn_req;
    conn_req.connection_id = MAGIC_CONNECTION_ID;
    conn_req.action        = CONNECTION_ACTION;

    while (true) {
        printf("Announcing to tracker (%s)\n", info->tr_host);
        conn_req.transaction_id = rand();
        send_connect_request(
            &conn_req, sock, (struct sockaddr*) info->tr_addr
        );

        ConnectResponse conn_resp;
        receive_connect_response(&conn_resp, sock);

        AnnounceRequest a_req;
        a_req.connection_id  = conn_resp.connection_id;
        a_req.action         = ANNOUNCE_ACTION;
        a_req.transaction_id = rand();
        memcpy(&a_req.info_hash, torrent->info_hash, 20);
        memcpy(&a_req.peer_id, client->peer_id, 20);
        a_req.downloaded = 0;
        a_req.left       = 0;
        a_req.uploaded   = 0;
        a_req.event      = ANNOUNCE_EVENT_NONE;
        // s_addr is in network order
        a_req.ip         = ntohl(client->ip->s_addr);
        a_req.num_want   = ANNOUNCE_NUM_WANT_DEFAULT;
        a_req.port       = client->port;
        a_req.extensions = ANNOUNCE_NO_EXTENSIONS;

        send_announce_request(
            &a_req, sock, (struct sockaddr*) info->tr_addr
        );

        AnnounceResponse a_resp;
        receive_announce_response(&a_resp, sock);
        

        PeerInfo *peer = a_resp.peers;
        int added = 0;
        while (peer) {
            if (add_peer(info->swarm, peer->ip, peer->port))
                added++;
            peer = peer->next;
        }
        printf("Tracker added %d peers (%s)\n", added, info->tr_host);
        printf(
            "Current peer count: %d\n",
            linked_list_len(info->swarm->peers)
        );

        sleep(60);
    }

    return NULL;
}

void init_trackers(Swarm *swarm) {
    int tcount = swarm->torrent->tracker_count;

    pthread_t threads[tcount];
    for (int i = 0; i < tcount; i++) {
        char *tracker = swarm->torrent->trackers[i];

        UriUriA tracker_uri;
        parse_uri(&tracker_uri, tracker);
        char *scheme = uri_scheme(&tracker_uri);
        if (strcmp(scheme, "udp") != 0) break;

        struct addrinfo *addr;
        char *host = uri_host(&tracker_uri);
        if (getaddrinfo(host, NULL, NULL, &addr) != 0)
            continue;

        struct sockaddr_in *sock_addr = (struct sockaddr_in*) addr->ai_addr;
        sock_addr->sin_family = AF_INET;
        sock_addr->sin_port   = htons(uri_port(&tracker_uri));

        struct tr_info *info = malloc(sizeof(struct tr_info));
        info->swarm   = swarm;
        info->tr_host = host;
        info->tr_addr = sock_addr;
        pthread_create(&threads[i], NULL, tracker_routine, info);
    }
    for (int i = 0; i < tcount; i++) {
        if (threads[i] != NULL)
            pthread_join(threads[i], NULL);
    }
}

void init_swarm(Swarm *swarm, Client *c, Torrent *t) {
    swarm->client  = c;
    swarm->torrent = t;
    swarm->peers   = malloc(sizeof(LinkedList));
    linked_list_new(swarm->peers, sizeof(Peer*));
    pthread_mutex_init(&(swarm->peer_mutex), NULL);

    init_trackers(swarm);
}
