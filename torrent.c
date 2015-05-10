#include "torrent.h"
#include "bencoding.h"
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
        } else if (strcmp(iter->key, "tr") == 0) {
            tracker_count++;
        }
        iter = iter->next;
    }

    char **trackers = malloc(tracker_count * sizeof(char*));
    iter = query;
    for (int i = 0; iter != 0; i++) {
        if (strcmp(iter->key, "tr") == 0) {
            trackers[i] = (char*) iter->value;
        }
        iter = iter->next;
    }

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
