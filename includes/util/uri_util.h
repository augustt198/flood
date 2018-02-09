#pragma once

#include "uriparser/Uri.h"

typedef UriUriA uri_t;
typedef UriQueryListA uri_query_list_t;

int parse_uri(uri_t *dst, char *str);

void uri_free_memebers(uri_t *uri);

char *uri_scheme(uri_t *uri);
char *uri_userinfo(uri_t *uri);
char *uri_host(uri_t *uri);
int uri_port(uri_t *uri);

int uri_query(uri_query_list_t **dst, uri_t *uri, int *items);

uri_query_list_t *uri_query_list_append(char *key, char *value,
    uri_query_list_t *prev);

int uri_set_query(uri_t *uri, uri_query_list_t *query);

// Converts `uri` to a string. Resulting string
// is stored in `dst`.
// Returns:
// the length of the uri string or
// -1 if an error occurred
int uri_to_string(uri_t *uri, char **dst);

UriTextRangeA str2textrange(char *str);
