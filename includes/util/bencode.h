#pragma once

#include "list.h"

typedef enum bencode_type {
    BENCODE_STRING,
    BENCODE_INTEGER,
    BENCODE_LIST,
    BENCODE_DICT
} bencode_type;

typedef char*       bencode_string;
typedef long long   bencode_integer;
typedef list_t      bencode_list;

// dicts are just a linked list
// of bencode_dict_entry
typedef list_t bencode_dict;

typedef struct bencode_value {
    bencode_type type;
    union {
      struct { bencode_string    string;  };
      struct { bencode_integer   integer; };
      struct { bencode_list      *list;   };
      struct { bencode_dict      *dict;   };
    };
} bencode_value;

int dict_lookup(bencode_dict *dict, char *key, bencode_value **dst);

typedef struct bencode_dict_entry {
    char *key;
    bencode_value *value;
} bencode_dict_entry;

int bencode_parse(char *string, int len, bencode_value *dst);
