#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tracker.h"

#include "torrent.h"
#include "util.h"
#include "udp_protocol.h"
#include "net.h"
#include "list.h"

int tracker_announce(tracker_t *t) {
    torrent_t *torrent = (torrent_t*) t->torrent;

    if (t->sock_fd == -1) {
        int res = create_udp_socket(t->url, &(t->sock_fd), &(t->addr));
        if (res != 0) {
            return res;
        }

        udpt_connect_req c_req;
        c_req.connection_id  = MAGIC_CONNECTION_ID;
        c_req.action         = CONNECTION_ACTION;
        c_req.transaction_id = rand();
        printf("Sending\n");
        send_connect_request(&c_req, t->sock_fd, t->addr);

        udpt_connect_resp c_resp;
        receive_connect_response(&c_resp, t->sock_fd);
        printf("received\n");

        if (c_resp.transaction_id != c_req.transaction_id) {
            debug("Bad transaction id received\n");
            return -1;
        } else {
            debug("Received connection response\n");
        }
        t->connection_id = c_resp.connection_id;
    }
    udpt_announce_req a_req = {0};
    a_req.connection_id     = t->connection_id;
    a_req.action            = ANNOUNCE_ACTION;
    a_req.transaction_id    = rand();

    memcpy(a_req.info_hash, torrent->info_hash, 20);
    memcpy(a_req.peer_id, "CUSTOMCLIENT12345678", 20);

    a_req.event      = ANNOUNCE_EVENT_STARTED;
    a_req.downloaded = 0;
    a_req.left       = 0;
    a_req.uploaded   = 0;
    a_req.ip         = 0;
    a_req.num_want   = ANNOUNCE_NUM_WANTED_DEFAULT;
    a_req.extensions = ANNOUNCE_NO_EXTENSIONS;

    printf("Announcing...\n");
    send_announce_request(&a_req, t->sock_fd, t->addr);
    printf("Done announcing...\n");

    udpt_announce_resp a_resp;
    receive_announce_response(&a_resp, t->sock_fd);

    udpt_peer *peer = a_resp.peers;
    while (peer != NULL) {
        if (t->find_fn != NULL) {
            discovered_peer_t p = {peer->ip, peer->port};
            t->find_fn(p, t->find_fn_handle);
            //printf("Found one\n");
        }
        peer = peer->next;
    }

    return 0;
}

void *tracker_update_routine(void *arg) {
    tracker_t *t = (tracker_t*) arg;
    printf("START\n");

    int tick = 0;
    while (!(t->stopped)) {
        if (tick % t->poll_freq == 0) {
            tracker_announce(t);
        }

        tick++;
        sleep(1);
    }
    printf("STOP\n");

    return NULL;
}

bool tracker_start(tracker_t *t) {
    t->sock_fd = -1;
    if (t->update_thread == NULL) {
        pthread_t *thread = malloc(sizeof(pthread_t));
        t->update_thread = thread;
        pthread_create(thread, NULL, tracker_update_routine, t);

        return true;
    } else {
        return false;
    }
}

bool tracker_stop(tracker_t *t, bool force) {
    if (t->stopped) {
        return false;
    } else {
        if (force) {
            pthread_cancel(*(t->update_thread));
        }
        t->stopped = true;
        return true;
    }
}
