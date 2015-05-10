#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

#include "linked_list.h"

typedef enum BencodeType {
    BENCODE_STRING,
    BENCODE_INTEGER,
    BENCODE_LIST,
    BENCODE_DICT
} BencodeType;

typedef char*       BencodeString;
typedef int         BencodeInteger;
typedef LinkedList  BencodeList;

// dicts are just a linked list
// of BencodeDictEntry
typedef LinkedList  BencodeDict;

typedef struct BencodeValue {
    BencodeType type;
    union {
      struct { BencodeString    string;  };
      struct { BencodeInteger   integer; };
      struct { BencodeList      *list;   };
      struct { BencodeDict      *dict;   };
    };
} BencodeValue;

typedef struct BencodeDictEntry {
    char *key;
    BencodeValue *value;
} BencodeDictEntry;


int bencode_parse(char *string, int len, BencodeValue *dst);

int b_parse_any(char *string, int len, int *pos, BencodeValue *dst);

int b_parse_string(char *string, int len, int *pos,
                   BencodeValue *dst, char first);

int b_parse_int(char *string, int len, int *pos, BencodeValue *dst);

int b_parse_list(char *string, int len, int *pos, BencodeValue *dst);

int b_parse_dict(char *string, int len, int *pos, BencodeValue *dst);

BencodeValue *dict_lookup(BencodeDict *dict, char *key);
