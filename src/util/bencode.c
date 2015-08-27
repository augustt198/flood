#include "bencode.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define EOF_CHECK(_pos, _len)\
    if (*_pos >= _len) return -1;

int b_parse_any    (char *string, int len, int *pos, bencode_value *dst);
int b_parse_string (char *string, int len, int *pos, bencode_value *dst, char first);
int b_parse_int    (char *string, int len, int *pos, bencode_value *dst);
int b_parse_list   (char *string, int len, int *pos, bencode_value *dst);
int b_parse_dict   (char *string, int len, int *pos, bencode_value *dst);


char *itoa(int n) {
    char *buf = malloc(19);
    memset(buf, 0, 19);
    sprintf(buf, "%d", n);
    return buf;
}

int bencode_parse(char *string, int len, bencode_value *dst) {
    int pos = 0;
    return b_parse_any(string, len, &pos, dst);
}

int b_parse_any(char *string, int len, int *pos, bencode_value *dst) {
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
                   bencode_value *dst, char first) {

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

    bencode_string bstr = {parsed_str, n};

    dst->type   = BENCODE_STRING;
    dst->string = bstr;

    return 0;
}

int b_parse_int(char *string, int len,
                int *pos, bencode_value *dst) {

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

int b_parse_list(char *string, int len, int *pos, bencode_value *dst) {
    EOF_CHECK(pos, len);
    list_t *list = malloc(sizeof(list_t));
    list_new(list, sizeof(bencode_value*));

    while (string[*pos] != 'e') {
        bencode_value *elem = malloc(sizeof(bencode_value));
        int code = b_parse_any(string, len, pos, elem);
        if (code != 0)
            return code;
        list_append(list, &elem);
        EOF_CHECK(pos, len);
    }

    *pos += 1;

    dst->type = BENCODE_LIST;
    dst->list = list;

    return 0;
}

int b_parse_dict(char *string, int len, int *pos, bencode_value *dst) {
    EOF_CHECK(pos, len);
    hashtable_t *dict = malloc(sizeof(hashtable_t));
    hashtable_init(dict, hash_func_str, cmp_func_str);

    while (string[*pos] != 'e') {
        bencode_value key;
        bencode_value *val = malloc(sizeof(bencode_value));

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

        hashtable_put(dict, key.string.ptr, val);
    }
    *pos += 1;

    dst->type = BENCODE_DICT;
    dst->dict = dict;
    return 0;
}

// number of digits in the decimal representation
// of `n` (including negative sign)
int declen(int n) {
    if (n == 0)
        return 1;
    int c = 0;
    if (n < 0) {
        c++;
        n = -n;
    }
    for (; n > 0; n /= 10)
        c++;

    return c;
}

// calculates length of a bencode value's
// string representation
int b_to_string_len(bencode_value *val) {
    bencode_type type = val->type;
    int len = 0;
    if (type == BENCODE_STRING) {
        int bstrlen = val->string.len;
        // <string len>:<string>
        len = declen(bstrlen) + 1 + bstrlen;
    } else if (type == BENCODE_INTEGER) {
        // i<number>e
        len = 1 + declen(val->integer) + 1;
    } else if (type == BENCODE_LIST) {
        int sum = 0;
        list_node *node = val->list->head;
        while (node != NULL) {
            bencode_value *elem = *((bencode_value**) node->data);
            int elem_len = b_to_string_len(elem);
            sum += elem_len;

            node = node->next;
        }
        // l<items>e
        len = 1 + sum + 1;
    } else if (type == BENCODE_DICT) {
        int sum = 0;
        hashtable_iter_t iter;
        hashtable_iterator(val->dict, &iter);
        while (hashtable_iter_has_next(&iter)) {
            void *key, *val;
            hashtable_iter_next(&iter, &key, &val);
            int key_len = strlen((char*) key);

            bencode_value *bval = (bencode_value*) val;
            int val_len = b_to_string_len(bval);

            // <key len>:<key><value>
            int pair_len = declen(key_len) + 1 + key_len + val_len;
            sum += pair_len;
        }
        // d<pairs>e 
        len = 1 + sum + 1;
    }

    return len;
}

void write_num(char *dst, int *i, int num) {
    char *str  = itoa(num);
    int numlen = strlen(str);
    memcpy(dst + *i, str, numlen);
    *i += numlen;
}

void b_to_string(char *dst, int *i, bencode_value *val) {
    bencode_type type = val->type;
    if (type == BENCODE_STRING) {
        char *prefix = itoa(val->string.len);
        int prefix_len = strlen(prefix);
        memcpy(dst + *i, prefix, prefix_len);
        *i += prefix_len;
        dst[*i] = ':';
        *i += 1;
        memcpy(dst + *i, val->string.ptr, val->string.len);
        *i += val->string.len;
    } else if (type == BENCODE_INTEGER) {
        dst[*i] = 'i';
        *i += 1;
        char *num = itoa(val->integer);
        memcpy(dst + *i, num, strlen(num));
        *i += strlen(num);
        dst[*i] = 'e';
        *i += 1;
    } else if (type == BENCODE_LIST) {
        dst[*i] = 'l';
        *i += 1;
        list_node *node = val->list->head;
        while (node != NULL) {
            bencode_value *elem = *((bencode_value**) node->data);
            b_to_string(dst, i, elem);
            
            node = node->next;
        }
        dst[*i] = 'e';
        *i += 1;
    } else if (type == BENCODE_DICT) {
        dst[*i] = 'd';
        *i += 1;

        int keys_len = hashtable_size(val->dict);
        char **keys = malloc(sizeof(char*) * keys_len);
        hashtable_iter_t iter;
        hashtable_iterator(val->dict, &iter);
        for (int j = 0; hashtable_iter_has_next(&iter); j++) {
            char *key;
            hashtable_iter_next(&iter, (void**) &key, NULL);
            keys[j] = key;
        }
        qsort(
            keys, keys_len, sizeof(char*),
            (int (*)(const void *, const void *)) strcmp
        );

        for (int j = 0; j < keys_len; j++) {
            char *key = keys[j];
            int key_len = strlen(key);
            write_num(dst, i, key_len);
            dst[*i] = ':';
            *i += 1;
            memcpy(dst + *i, key, key_len);
            *i += key_len;

            bencode_value *pair_val;
            hashtable_get(val->dict, key, (void**) &pair_val);
            
            b_to_string(dst, i, pair_val);
        }
        
        dst[*i] = 'e';
        *i += 1;
    }
}

int bencode_to_string(char **strp, bencode_value *val) {
    int len   = b_to_string_len(val);
    char *str = malloc(len + 1);
    int i     = 0;
    b_to_string(str, &i, val);

    *strp = str;
    return len;
}

