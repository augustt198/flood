#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <argp.h>

#include "bencode.h"
#include "torrent.h"
#include "client.h"

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

static bool continueRunning = true;
void interruptHandler(int _) {
    continueRunning = false;
}

void setup() {
    srand(time(NULL));
    // don't kill process
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, interruptHandler);
}

client_config_t default_config();

int main(int argc, char **argv) {
    setup();

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
    for (int i = 0; i < torrent.tracker_count && i < 10; i++) {
        printf("%s\n", torrent.tracker_urls[i]);
    }

    printf("File name: %s\n", torrent.info.file_name);
    if (torrent.info.mode == MULTI_FILE_MODE) {
        printf("Files (%d):\n", torrent.info.file_count);
        for (int i = 0; i < torrent.info.file_count; i++) {
            file_info_t f = torrent.info.files[i];
            printf(" * ");
            for (int j = 0; j < f.path_segments; j++) {
                printf("%s", f.path[j]);
                if (j < f.path_segments - 1)
                    printf("/");
            }
            printf("\n");
        }
    }

    client_t client;
    client_init(&client, default_config(), &torrent);

    client_init_trackers(&client);
    client_start_trackers(&client);
    client_start_peer_threads(&client);
    while (continueRunning) {
        sleep(2);

        int unchecked = 0;
        list_iter_start_safe(client.peers);
        while (list_iter_has_next(client.peers)) {
            peer_t *p;
            list_iter_next(client.peers, (void*) &p);
            if (p->status == PEER_STATUS_NEW)
                unchecked++;
        }
        list_iter_stop_safe(client.peers);
        printf("%d peers, %d unchecked\n", list_len(client.peers), unchecked);
    }
    printf("Stopping!\n");
    client_stop_trackers(&client, false);

    sleep(2); // give people time to wrap up
    printf("Goodbye\n");

    return 0;
}

client_config_t default_config() {
    client_config_t cfg;
    cfg.tracker_poll_frequency = 30;
    cfg.peer_threads = 20;
    cfg.peer_timeout = 3;
    memcpy(cfg.peer_id, "CUSTOMCLIENT12345678", 20);

    return cfg;
}
