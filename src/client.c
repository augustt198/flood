#include "client.h"

#include <stdio.h>

void client_init(client_t *c, client_config_t config, torrent_t *t) {
    c->torrent = t;
    c->config = config;
    c->peers = malloc(sizeof(list_t));
    list_new(c->peers, sizeof(peer_t));
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

    peer_t peer;
    peer.ip_address = new_peer.ip;
    peer.port       = new_peer.port;
    list_append(c->peers, &peer);
}
