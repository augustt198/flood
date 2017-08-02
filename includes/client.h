#pragma once

#include "torrent.h"
#include "tracker.h"
#include "peer.h"
#include "list.h"

typedef struct client_config {
    int tracker_poll_frequency;
} client_config_t;

typedef struct client {
    list_t *peers;
    torrent_t *torrent;
    client_config_t config;

    tracker_t *trackers;
} client_t;

void client_init(client_t *c, client_config_t config, torrent_t *t);
void client_init_trackers(client_t *c);
void client_start_trackers(client_t *c);
void client_stop_trackers(client_t *c, bool force);

void client_find_peer(discovered_peer_t, void *handle);
