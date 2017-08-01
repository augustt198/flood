#include "torrent.h"

#include "bencode.h"
#include "list.h"
#include "util.h"

#include "peer_protocol.h"

#include <stdlib.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define ERR_EXIT(msg) \
    do { fputs(msg, stderr); return -1; } while (0);

// in seconds
#define TRACKER_POLL_FREQ 300

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

struct peer_thread_message {
    peer_t peer;
    torrent_t *torrent;
};

int total = 0;

void *peer_ping_routine(void *handle) {
    if (total++ > 50) return NULL;

    struct peer_thread_message *msg = (struct peer_thread_message*) handle;
    peer_t peer = msg->peer;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(peer.port);
    printf(">>>> PEER PORT %d\n", peer.port);
    addr.sin_addr.s_addr = htonl(peer.ip);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("COULDNT OPEN SOCKET: %s\n", strerror(errno));
    }
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    peer_handshake handshake;
    handshake.pstrlen = PEER_PROTOCOL_LEN;
    handshake.pstr = "BitTorrent protocol";
    memcpy(handshake.info_hash, msg->torrent->info_hash, 20);
    //memcpy(handshake.peer_id, "DAB ON THE HATERS!!", 20);
    memcpy(handshake.peer_id, "-UT1870-123456789012", 20);

    ssize_t s = send_handshake_request(&handshake, sock, (struct sockaddr*) &addr);
    if (s < 0) {
        printf("SEND ERROR: %s\n", strerror(errno));
        close(sock);
        return NULL;
    }

    peer_handshake response;
    ssize_t s2 = receive_handshake_request(&response, sock, (struct sockaddr*) &addr, PEER_PROTOCOL_LEN);
    if (s2 < 67) {
        close(sock);
        return NULL;
    }
    printf("got response size %zd /// %d /// %s\n", s2, response.pstrlen, response.pstr);
    printf("RESPONSE PEER ID: %.*s\n", 20, response.peer_id);

    char lenPrefixBuf[4];
    recv(sock, lenPrefixBuf, 4, 0);
    char msgType;
    recv(sock, &msgType, 1, 0);
    printf("*********** GOT MESSAGE TYPE %hhu\n", msgType);
    if (msgType == 5) {
        int bitfieldLen = ntohl(*((uint32_t*) lenPrefixBuf)) - 1;
        char bitfield[bitfieldLen];
        recv(sock, bitfield, bitfieldLen, 0);
        printf_lock();
        printf("BITFIELD ");
        for (int i = 0; i < bitfieldLen; i++) {
            for (int j = 0; j < 8; j++) {
                printf("%d", (bitfield[i]>>j)&1);
            }
        }
        printf("\n");
        printf_unlock();
    }

    free(msg);
    return NULL;
}

void tracker_find_peer(peer_t peer, void *handle) {
    torrent_t *t = (torrent_t*) handle;
    char ipstr[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &peer.ip, ipstr, INET_ADDRSTRLEN);

    //debug("Found peer: %s:%d\n", ipstr, peer.port);

    struct peer_thread_message *msg = calloc(1, sizeof(struct peer_thread_message));
    msg->peer = peer;
    msg->torrent = t;
    pthread_t thread;
    pthread_create(&thread, NULL, peer_ping_routine, msg);
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
