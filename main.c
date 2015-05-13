#include <stdio.h>
#include <stdlib.h>

#include "uri_util.h"

void die(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void handle_magnet(char *magnet);

int main(int argc, char **argv) {
    if (argc < 3) {
        die("Expected at least two arguments\n");
    }

    if (strcmp(argv[1], "magnet") == 0) {
        handle_magnet(argv[2]);
        return 0;
    } else if (strcmp(argv[1], "torrent") == 0) {
        die("Not supported yet\n");
    } else {
        fprintf(stderr, "Unknown option: %s\n", argv[1]);
        exit(1);
    }
}

void handle_magnet(char *magnet) {
    UriUriA uri;
    if (parse_uri(&uri, magnet) != URI_SUCCESS) {
        fprintf(stderr, "Malformed magnet link\n");
        exit(1);
    }

    int query_count;
    UriQueryListA *query;
    if (uri_query(&query, &uri, &query_count) != URI_SUCCESS) {
        die("Malformed query string\n");
    }

    char *scheme = uri_scheme(&uri);
    if (strcmp(scheme, "magnet") != 0) {
        die("Expected magnet protocol\n");
        exit(1);
    }

    // todo more stuff
}
