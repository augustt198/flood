#include "torrent.h"

#include "bencode.h"
#include "list.h"
#include "util.h"

#include <stdlib.h>
#include <openssl/sha.h>
#include <arpa/inet.h>

#define ERR_EXIT(msg) \
    do { fputs(msg, stderr); return -1; } while (0);

#define TRACKER_POLL_FREQ 30

void prepare_trackers(bencode_value *bencode, torrent_t *t);

int prepare_info_section(bencode_value *bencode, torrent_t *t);

int torrent_init_from_file(char *filepath, torrent_t *t) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Could not open torrent file");
        return -1;
    }

    char *data;
    long data_len = read_file(file, &data);
    if (data_len < 0)
        ERR_EXIT("Unable to read file data\n");

    bencode_value bencode;
    if (bencode_parse(data, data_len, &bencode) != 0)
        ERR_EXIT("Invalid torrent file (bad bencode)\n");
    
    bencode_value *info_sect;
    if (!hashtable_get(bencode.dict, "info", (void**) &info_sect))
        ERR_EXIT("Info section not found\n");

    char hash[SHA_DIGEST_LENGTH];
    SHA1(
        (unsigned char*) data + info_sect->start,
        info_sect->end - info_sect->start + 1,
        (unsigned char*) hash
    );

    memcpy(t->info_hash, hash, 20);

    prepare_trackers(&bencode, t);
    prepare_info_section(&bencode, t);
    
    return 0;
}

void prepare_trackers(bencode_value *bencode, torrent_t *t) {
    int tracker_count = 0;
    bencode_value *announce;
    bencode_value *announce_list;

    char **tracker_urls = NULL;

    if (hashtable_get(bencode->dict, "announce-list", (void**) &announce_list)) {
        tracker_count = list_len(announce_list->list);
        tracker_urls = malloc(sizeof(char*) * (tracker_count + 1));
        
        list_iter_start(announce_list->list);
        for (int i = 0; list_iter_has_next(announce_list->list); i++) {
            bencode_value *b_sublist;
            list_iter_next(announce_list->list, (void*) &b_sublist);
            list_t *sublist = b_sublist->list;

            bencode_value *tracker;
            list_get(sublist, 0, (void*) &tracker);
            tracker_urls[i] = tracker->string.ptr;
        }

    } else {
        tracker_urls = malloc(sizeof(char*));
    }
  
    if (hashtable_get(bencode->dict, "announce", (void**) &announce)) {
        char *announce_str = announce->string.ptr;
        tracker_urls[tracker_count] = announce_str;
        tracker_count++;
    } 

    t->tracker_count = tracker_count;
    t->tracker_urls  = tracker_urls;
}

// Loads info section of `bencode` into `t`
// returns: 0 if successful
int prepare_info_section(bencode_value *bencode, torrent_t *t) {
    info_section_t *info = &(t->info);

    bencode_value *b_info;
    if (!hashtable_get(bencode->dict, "info", (void**) &b_info))
        return -1;

    bencode_value *b_name;
    if (hashtable_get(b_info->dict, "name", (void**) &b_name)) {
        info->file_name = b_name->string.ptr;
    }
    bencode_value *b_files;
    if (hashtable_get(b_info->dict, "files", (void**) &b_files)) {
        info->mode = MULTI_FILE_MODE;
    } else {
        info->mode = SINGLE_FILE_MODE;
    }

    return 0;
}

void tracker_find_peer(peer_t peer, void *handle) {
    torrent_t *t = (torrent_t*) handle;
    char ipstr[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &peer.ip, ipstr, INET_ADDRSTRLEN);

    debug("Found peer: %s:%d\n", ipstr, peer.port);
}

void start_trackers(torrent_t *t) {
    tracker_t *prev = NULL;
    for (int i = 0; i < t->tracker_count; i++) {
        char *tracker_url = t->tracker_urls[i];
        
        tracker_t *tracker = calloc(1, sizeof(tracker_t));
        tracker->url       = tracker_url;
        tracker->handle    = t;
        tracker->poll_freq = TRACKER_POLL_FREQ;
        tracker->find_fn   = tracker_find_peer;

        tracker_start(tracker);

        if (prev == NULL) {
            t->trackers = tracker;
        } else {
            prev->next = tracker;
        }
        prev = tracker;
    }
}
