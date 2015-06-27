#include "torrent.h"
#include "uri_util.h"

int magnet2torrent(Torrent* dst, char *magnet) {
    UriUriA uri;

    int code = parse_uri(&uri, magnet);
    if (code != URI_SUCCESS) {
        uriFreeUriMembersA(&uri);
        return code;
    }

    UriQueryListA *query;
    code = uri_query(&query, &uri, 0);
    if (code != URI_SUCCESS) {
        uriFreeQueryListA(query);
        uriFreeUriMembersA(&uri);
        return code;
    }

    UriQueryListA *iter = query;
    int tracker_count  = 0;
    while (iter != 0) {
        if (strcmp(iter->key, "dn") == 0) {
            dst->filename = (char*) iter->value;
        } else if (strcmp(iter->key, "xt") == 0) {
            char *hash = strdup(iter->value + 9);
            char *info_hash = malloc(20);
            hexhash2bin(hash, 40, info_hash);
            dst->info_hash = info_hash;
        } else if (strcmp(iter->key, "tr") == 0) {
            tracker_count++;
        }
        iter = iter->next;
    }
    dst->tracker_count = tracker_count;

    char **trackers = malloc(tracker_count * sizeof(char*));
    iter = query;
    for (int i = 0; iter != 0;) {
        if (strcmp(iter->key, "tr") == 0) {
            trackers[i] = (char*) iter->value;
            i++;
        }
        iter = iter->next;
    }

    dst->trackers = trackers;

    return 0;
}

int bencode2torrent(Torrent *dst, char *bencode) {
    BencodeValue val;
    int code = bencode_parse(bencode, strlen(bencode), &val);
    if (code != 0)
        return code;
    if (val.type != BENCODE_DICT)
        return -1;

    BencodeValue *announce = dict_lookup(val.dict, "announce");
    if (announce == 0 || announce->type != BENCODE_STRING)
        return -1;

    dst->trackers      = malloc(sizeof(char*));
    dst->trackers[0]   = announce->string;
    dst->tracker_count = 0;

    return 0;
}

char hexdig2dec(char chr) {
    if (chr >= '0' && chr <= '9')
        return chr - '0';
    else if (chr >= 'a' && chr <= 'f')
        return chr - 'a' + 10;
    else if (chr >= 'A' && chr <= 'F')
        return chr - 'A' + 10;
    else
        return 0;
}

void hexhash2bin(char *hash, int hlen, char *bin) {
    for (int i = 0, k = 0; i < hlen; i += 2, k++) {
        bin[k] = (hexdig2dec(hash[i]) << 4) +
                    hexdig2dec(hash[i + 1]);
    }
}
