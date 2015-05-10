#include <stdio.h>
#include <stdlib.h>

#include "uri_util.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Expected at least one argument\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        UriUriA uri;
        if (parse_uri(&uri, argv[i]) != URI_SUCCESS) {
            printf("failure\n");
            uriFreeUriMembersA(&uri);
            exit(1);
        }

        int count;
        UriQueryListA *query;

        if (uri_query(&query, &uri, &count) != URI_SUCCESS) {
            printf("failure\n");
            uriFreeQueryListA(query);
            uriFreeUriMembersA(&uri);
            exit(1);

        }

        while (query != 0) {
            printf("key: %s\n", query->key);
            printf("value: %s\n", query->value);
            printf("---------------\n");
            query = query->next;
        }
        uriFreeQueryListA(query);
        uriFreeUriMembersA(&uri);
    }
}
