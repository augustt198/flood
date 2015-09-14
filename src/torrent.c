#include "torrent.h"

#include "bencode.h"
#include "list.h"
#include "util.h"

#include <stdlib.h>
#include <openssl/sha.h>

#define ERR_EXIT(msg) \
    do { fputs(msg, stderr); return -1; } while (0);

void prepare_trackers(bencode_value *bencode, torrent_t *t);

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
    
    return 0;
}

void prepare_trackers(bencode_value *bencode, torrent_t *t) {
    int tracker_count = 0;
    bencode_value *announce;
    bencode_value *announce_list;

    char **trackers = NULL;

    if (hashtable_get(bencode->dict, "announce-list", (void**) &announce_list)) {
        tracker_count = list_len(announce_list->list);
        trackers = malloc(sizeof(char*) * (tracker_count + 1));
        
        list_iter_start(announce_list->list);
        for (int i = 0; list_iter_has_next(announce_list->list); i++) {
            bencode_value *b_sublist;
            list_iter_next(announce_list->list, (void*) &b_sublist);
            list_t *sublist = b_sublist->list;

            bencode_value *tracker;
            list_get(sublist, 0, (void*) &tracker);
            trackers[i] = tracker->string.ptr;
        }

    } else {
        trackers = malloc(sizeof(char*));
    }
  
    if (hashtable_get(bencode->dict, "announce", (void**) &announce)) {
        char *announce_str = announce->string.ptr;
        trackers[tracker_count] = announce_str;
        tracker_count++;
    } 

    t->tracker_count = tracker_count;
    t->trackers      = trackers;
}
