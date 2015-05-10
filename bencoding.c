#include "bencoding.h"

#define EOF_CHECK(_pos, _len)\
    if (*_pos >= _len) return -1;

int bencode_parse(char *string, int len, BencodeValue *dst) {
    int pos = 0;
    return b_parse_any(string, len, &pos, dst);
}

int b_parse_any(char *string, int len, int *pos, BencodeValue *dst) {
    EOF_CHECK(pos, len);
    char prefix = string[*pos];
    *pos += 1;
    switch (prefix) {
        case 'i':
            return b_parse_int(string, len, pos, dst);
        case 'l':
            return b_parse_list(string, len, pos, dst);
        case 'd':
            return b_parse_dict(string, len, pos, dst);
        default:
            if (isdigit(prefix)) {
                return b_parse_string(string, len, pos, dst, prefix);
            } else {
                return -2;
            }
    }
    
    // ok
    return 0;
}

int b_parse_string(char *string, int len, int *pos,
                   BencodeValue *dst, char first) {

    int n = first - '0';
    while (isdigit(string[*pos])) {
        n = n * 10 + (string[*pos] - '0');
        *pos += 1;
        EOF_CHECK(pos, len);
    }

    if (string[*pos] != ':')
        return -2;
    *pos += 1;

    // add 1 to include \0 at end
    char *parsed_str = malloc(n + 1);
    for (int i = 0; i < n; i++) {
        EOF_CHECK(pos, len);
        parsed_str[i] = string[*pos];
        *pos += 1;
    }

    dst->type   = BENCODE_STRING;
    dst->string = parsed_str;

    return 0;
}

int b_parse_int(char *string, int len,
                int *pos, BencodeValue *dst) {

    EOF_CHECK(pos, len);

    bool neg = false;
    if (string[*pos] == '-') {
        neg = true;
        *pos += 1;
    }

    int num = 0;
    while (isdigit(string[*pos])) {
        num = num * 10 + (string[*pos] - '0');
        *pos += 1;
        EOF_CHECK(pos, len);
    }
    if (neg)
        num *= -1;

    if (string[*pos] != 'e') {
        return -2;
    }
    *pos += 1;

    dst->type = BENCODE_INTEGER;
    dst->integer = num;

    return 0;
}

int b_parse_list(char *string, int len, int *pos, BencodeValue *dst) {
    EOF_CHECK(pos, len);
    LinkedList *list = malloc(sizeof(LinkedList));
    linked_list_new(list, sizeof(BencodeValue*));

    while (string[*pos] != 'e') {
        BencodeValue *elem = malloc(sizeof(BencodeValue));
        int code = b_parse_any(string, len, pos, elem);
        linked_list_append(list, &elem);
        EOF_CHECK(pos, len);
    }

    *pos += 1;

    dst->type = BENCODE_LIST;
    dst->list = list;

    return 0;
}

int b_parse_dict(char *string, int len, int *pos, BencodeValue *dst) {
    EOF_CHECK(pos, len);
    LinkedList *dict = malloc(sizeof(LinkedList));
    linked_list_new(dict, sizeof(BencodeDictEntry*));

    while (string[*pos] != 'e') {
        BencodeDictEntry *entry = malloc(sizeof(BencodeDictEntry));
        BencodeValue key;
        BencodeValue *val = malloc(sizeof(BencodeValue));

        int code = b_parse_any(string, len, pos, &key);
        if (code != 0)
            return code;
        EOF_CHECK(pos, len);
        code = b_parse_any(string, len, pos, val);
        if (code != 0)
            return code;
        EOF_CHECK(pos, len);

        if (key.type != BENCODE_STRING)
            return -2;
        entry->key   = key.string;
        entry->value = val;
        linked_list_append(dict, &entry);

    }
    *pos += 1;

    dst->type = BENCODE_DICT;
    dst->dict = dict;
    return 0;
}

BencodeValue *dict_lookup(BencodeDict *dict, char *key) {
    iter_start(dict);
    while (iter_has_next(dict)) {
        BencodeDictEntry *entry;
        iter_next(dict, &entry);
        if (strcmp(entry->key, key) == 0)
            return entry->value;
    }

    return 0;
}
