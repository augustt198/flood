#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "torrent.h"

typedef struct discovered_peer {
    int ip;
    int port;
} discovered_peer_t;

typedef void (*tracker_stop_fn)(bool graceful);

typedef void (*tracker_find_peer_fn)(discovered_peer_t peer, void *handle);

typedef struct tracker {
    char *url;
    tracker_stop_fn stop_fn;

    torrent_t *torrent;
    
    // passed as parameter to find_fn
    void *find_fn_handle;
    tracker_find_peer_fn find_fn;

    // polling frequency (seconds)
    int poll_freq;

    pthread_t *update_thread;
    bool stopped;

    int sock_fd;
    struct sockaddr *addr;
    int64_t connection_id;

    struct tracker *next;
} tracker_t;

bool tracker_start(tracker_t *t);
bool tracker_stop(tracker_t *t, bool force);
