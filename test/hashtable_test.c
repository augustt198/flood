#include "hashtable.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *randstr() {
    int len   = 20 + rand() % 40;
    char *str = calloc(1, len + 1);

    str[len] = '\0';

    for (int i = 0; i < len; i++) {
        str[i] = 'a' + (rand() % 26);
    }

    return str;
}

#define SIZE 100000

void test_strings();

int main() {
    test_strings();

    printf("All assertions passed!\n");
}

void test_strings() {
    srand(time(NULL));
    hashtable_t map;

    hashtable_init(&map, hash_func_str, cmp_func_str);    

    char *keys[SIZE];
    char *vals[SIZE];
    for (int i = 0; i < SIZE; i++) {
        keys[i] = randstr();
        vals[i] = randstr();

        assert(hashtable_put(&map, keys[i], vals[i]) == 0);
    }

    assert(hashtable_size(&map) == SIZE);

    for (int i = 0; i < SIZE; i++) {
        char *res;
        assert(hashtable_get(&map, keys[i], (void**) &res) == 1);

        assert(strcmp(res, vals[i]) == 0);
    }

    hashtable_iter_t iter;
    hashtable_iterator(&map, &iter);

    int count = 0;
    while (hashtable_iter_has_next(&iter)) {
        char *key;
        char *val;
        hashtable_iter_next(&iter, (void**) &key, (void**) &val);

        count++;
    }
    assert(count == SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        char *res;
        assert(hashtable_remove(&map, keys[i], (void**) &res) == 1);

        assert(strcmp(res, vals[i]) == 0);
    }

    assert(hashtable_size(&map) == 0);
}
