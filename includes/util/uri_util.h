#pragma once

#include "uriparser/Uri.h"

typedef UriUriA uri_t;
typedef UriQueryListA uri_query_list_t;

int parse_uri(uri_t *dst, char *str);

char *uri_scheme(uri_t *uri);
char *uri_userinfo(uri_t *uri);
char *uri_host(uri_t *uri);
int uri_port(uri_t *uri);

int uri_query(uri_query_list_t **dst, uri_t *uri, int *items);
