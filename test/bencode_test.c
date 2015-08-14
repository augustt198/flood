#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bencode.h"

int main() {
    /* test integers */
    char *str = "i123e";

    bencode_value val;
    int status = bencode_parse(str, strlen(str), &val);
    assert(status       == 0);
    assert(val.type     == BENCODE_INTEGER);
    assert(val.integer  == 123);

    /* test byte strings */
    str = "5:hello";

    status = bencode_parse(str, strlen(str), &val);
    assert(status   == 0);
    assert(val.type == BENCODE_STRING);
    assert(strcmp(val.string.ptr, "hello") == 0);

    /* test lists */
    str = "l3:foo3:bari123ee";

    status = bencode_parse(str, strlen(str), &val);
    assert(status   == 0);
    assert(val.type == BENCODE_LIST);
    assert(val.list->len == 3);

    bencode_value *list_elem;

    list_get(val.list, 0, &list_elem);
    assert(list_elem->type == BENCODE_STRING);
    assert(strcmp(list_elem->string.ptr, "foo") == 0);

    list_get(val.list, 1, &list_elem);
    assert(list_elem->type == BENCODE_STRING);
    assert(strcmp(list_elem->string.ptr, "bar") == 0);

    list_get(val.list, 2, &list_elem);
    assert(list_elem->type      == BENCODE_INTEGER);
    assert(list_elem->integer   == 123);

    /* test dicts */
    str = "d3:fooi123ee";

    status = bencode_parse(str, strlen(str), &val);
    assert(status == 0);
    assert(val.type == BENCODE_DICT);
    assert(val.dict->len == 1);

    bencode_dict_entry *entry;
    list_get(val.dict, 0, &entry);

    assert(strcmp(entry->key, "foo") == 0);
    assert(entry->value->type == BENCODE_INTEGER);
    assert(entry->value->integer == 123);

    printf("All assertions passed!\n");
}
