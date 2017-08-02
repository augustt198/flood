#pragma once

#include "torrent.h"
#include "tracker.h"
#include "peer.h"
#include "list.h"

typedef struct client_config {
    int tracker_poll_frequency; // in seconds
    int peer_threads;
    int peer_timeout; // in seconds
    char peer_id[20];
} client_config_t;

typedef struct client {
    list_t *peers; // list of peer_t*
    torrent_t *torrent;
    client_config_t config;

    tracker_t *trackers;
} client_t;

void client_init(client_t *c, client_config_t config, torrent_t *t);
void client_init_trackers(client_t *c);
void client_start_trackers(client_t *c);
void client_stop_trackers(client_t *c, bool force);
void client_start_peer_threads(client_t *c);


void client_find_peer(discovered_peer_t, void *handle);
