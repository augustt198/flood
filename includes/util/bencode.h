#pragma once

#include "list.h"

typedef enum BencodeType {
    BENCODE_STRING,
    BENCODE_INTEGER,
    BENCODE_LIST,
    BENCODE_DICT
} BencodeType;

typedef char*       BencodeString;
typedef long long   BencodeInteger;
typedef List        BencodeList;

// dicts are just a linked list
// of BencodeDictEntry
typedef List BencodeDict;

typedef struct BencodeValue {
    BencodeType type;
    union {
      struct { BencodeString    string;  };
      struct { BencodeInteger   integer; };
      struct { BencodeList      *list;   };
      struct { BencodeDict      *dict;   };
    };
} BencodeValue;

int dict_lookup(BencodeDict *dict, char *key, BencodeValue **dst);

typedef struct BencodeDictEntry {
    char *key;
    BencodeValue *value;
} BencodeDictEntry;

int bencode_parse(char *string, int len, BencodeValue *dst);
