#pragma once

#include "torrent.h"
#include "tracker.h"

typedef struct client {
    torrent_t *torrent;
    list_t    *peers;

    tracker_t *tracker;
} client_t;
