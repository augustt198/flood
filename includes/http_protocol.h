#pragma once

#include <stdbool.h>
#include "util/uri_util.h"

typedef struct http_tracker_request {
    char info_hash[20];
    char peer_id[20];
    int port;
    int uploaded, downloaded, left;
    bool compact;
    bool no_peer_id;
    char *ip;
    int numwant;
    char *key;
    char *trackerid;
} http_tracker_request_t;

typedef struct http_tracker_response {
    char *failure_reason;
} http_tracker_response_t;

int http_tracker_request(uri_t uri, http_tracker_request_t *req, http_tracker_response_t *res);

void http_tracker_response_free_field(http_tracker_response_t *t);
