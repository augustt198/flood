#include <stdlib.h>
#include <stdio.h>

#include <argp.h>

#include "bencode.h"
#include "torrent.h"

void die(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void usage() {
    die("Usage: file [torrent file]\n");
}

void process_torrent_file(char *fp);

const char *argp_program_version = "Flood v0.1";
static char doc[] = "A BitTorrent client.";
static char args_doc[] = "[options]";
static struct argp_option options[] = {
    { "torrent", 't', "file", 0, "Download using a .torrent file." },
    { "magnet",  'm', "link", 0, "Download using a magnet link. " },
    { 0 },
};

struct arguments {
    enum {
        UNKNOWN_MODE, TORRENT_FILE_MODE, MAGNET_LINK_MODE
    } mode;
    union {
        char *torrent_file;
        char *magnet_link;
    };
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = state->input;
    switch (key) {
        case 't':
            if (args->mode != UNKNOWN_MODE)
                argp_usage(state);
            args->mode         = TORRENT_FILE_MODE;
            args->torrent_file = arg;
            break;
        case 'm':
            if (args->mode != UNKNOWN_MODE)
                argp_usage(state);
            args->mode        = MAGNET_LINK_MODE;
            args->magnet_link = arg;
            break;
        case ARGP_KEY_END:
            if (args->mode == UNKNOWN_MODE)
                argp_usage(state);
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv) {
    struct arguments args = {UNKNOWN_MODE, {0}};

    argp_parse(&argp, argc, argv, 0, 0, &args);

    torrent_t torrent;
    if (args.mode == TORRENT_FILE_MODE) {
        int status = torrent_init_from_file(args.torrent_file, &torrent);
        if (status != 0) {
            printf("Aborting due to errors\n");
            return status;
        }
    } else if (args.mode == MAGNET_LINK_MODE) {
        printf("Magnet link: %s\n", args.magnet_link);
    }

    printf("Torrent trackers (%d):\n", torrent.tracker_count);
    for (int i = 0; i < torrent.tracker_count; i++) {
        printf("%s\n", torrent.trackers[i]);
    }

    printf("File name: %s\n", torrent.info.file_name);

    return 0;
}
