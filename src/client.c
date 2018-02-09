#include "client.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#include "peer_protocol.h"
#include "util.h"

void client_init(client_t *c, client_config_t config, torrent_t *t) {
    c->torrent = t;
    c->config = config;
    c->peers = malloc(sizeof(list_t));
    list_new(c->peers, sizeof(peer_t*));
}

void client_init_trackers(client_t *c) {
    tracker_t *prev = NULL;
    for (int i = 0; i < c->torrent->tracker_count; i++) {
        char *tracker_url = c->torrent->tracker_urls[i];

        tracker_t *tracker      = calloc(1, sizeof(tracker_t));
        tracker->url            = tracker_url;
        tracker->poll_freq      = c->config.tracker_poll_frequency;
        tracker->find_fn_handle = c;
        tracker->find_fn        = client_find_peer;
        tracker->torrent        = c->torrent;

        if (prev == NULL) {
            c->trackers = tracker;
        } else {
            prev->next = tracker;
        }
        prev = tracker;
    }
}

void client_start_trackers(client_t *c) {
    tracker_t *tracker = c->trackers;

    while (tracker != NULL) {
        tracker_start(tracker);
        tracker = tracker->next;
    }
}

void client_stop_trackers(client_t *c, bool force) {
    tracker_t *tracker = c->trackers;
    while (tracker != NULL) {
        tracker_stop(tracker, force);
        tracker = tracker->next;
    }
}

void client_find_peer(discovered_peer_t new_peer, void *handle) {
    client_t *c = (client_t*) handle;

    list_iter_start_safe(c->peers);
    while (list_iter_has_next(c->peers)) {
        peer_t peer;
        list_iter_next(c->peers, &peer);

        if (peer.ip_address == new_peer.ip) {
            list_iter_stop_safe(c->peers);
            return;
        }
    }
    list_iter_stop_safe(c->peers);

    peer_t *peer = malloc(sizeof(peer_t));
    peer->ip_address = new_peer.ip;
    peer->port       = new_peer.port;
    peer->choking    = false;
    peer->interested = false;
    list_append(c->peers, &peer);
}

void *peer_thread_routine(void *handle);

void client_start_peer_threads(client_t *c) {
    for (int i = 0; i < c->config.peer_threads; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, peer_thread_routine, c);
    }
}

void peer_communication(peer_t *peer, client_t *client) {
    struct sockaddr_in addr;
    addr.sin_family     = AF_INET;
    addr.sin_port       = htons(peer->port);
    addr.sin_addr.s_addr = htonl(peer->ip_address);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        peer->status = PEER_STATUS_BAD;
        return;
    }
    struct timeval timeout_tv;
    timeout_tv.tv_sec = client->config.peer_timeout;
    timeout_tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_tv, sizeof(timeout_tv));

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        peer->status = PEER_STATUS_BAD;
        printf("bad peer yo\n");
        return;
    }

    peer_handshake handshake_out;
    handshake_out.pstrlen  = PEER_PROTOCOL_LEN;
    handshake_out.pstr     = PEER_PROTOCOL_STR;
    handshake_out.reserved = 0;

    memcpy(handshake_out.peer_id, client->config.peer_id, 20);
    memcpy(handshake_out.info_hash, client->torrent->info_hash, 20);

    send_peer_handshake(&handshake_out, sock, (struct sockaddr*) &addr);

    peer_handshake handshake_in;
    receive_peer_handshake(&handshake_in, sock, (struct sockaddr*) &addr);

    bool good;
    good = memcmp(handshake_out.info_hash, handshake_in.info_hash, 20) == 0;
    good = good && (handshake_in.pstrlen == PEER_PROTOCOL_LEN);
    good = good && (memcmp(handshake_in.pstr, PEER_PROTOCOL_STR, PEER_PROTOCOL_LEN) == 0);
    if (!good) {
        peer->status = PEER_STATUS_BAD;
        return;
    }

    printf("Good peer!!!!!!!!!\n");

    char *peer_name = NULL;
    peer_id_friendly(handshake_in.peer_id, &peer_name);
    if (peer_name == NULL) peer_name = "Unknown";

    printf("Connected to peer: %s\n", peer_name);

    while (true) {
        peer_message msg;
        receive_peer_message(&msg, sock, (struct sockaddr*) &addr);
        printf("[%s] ", peer_name);
        print_peer_message(&msg);
        if (msg.type == PEER_MSG_HAVE) {
            printf("HAVE PIECE: index: %u\n", msg.msg_have.piece_index);
        }
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000;
        nanosleep(&ts, NULL);
    }

}

void *peer_thread_routine(void *handle) {
    client_t *c = (client_t*) handle;
    while (true) {
        if (list_len(c->peers) == 0) {
            sleep(1);
            continue;
        }

        list_iter_start_safe(c->peers);
        peer_t *selected = NULL;
        while (list_iter_has_next(c->peers)) {
            peer_t *p;
            list_iter_next(c->peers, &p);
            if (p->status == PEER_STATUS_NEW) {
                selected = p;
                break;
            }
        }
        list_iter_stop_safe(c->peers);
        if (selected != NULL) {
            selected->status = PEER_STATUS_IN_USE;
            peer_communication(selected, c);
        } else {
            sleep(1);
        }
        printf("im still alive\n");

    }

    return NULL;
}
