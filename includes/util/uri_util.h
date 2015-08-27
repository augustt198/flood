#pragma once

#include "uriparser/Uri.h"

int parse_uri(UriUriA *dst, char *str);

char *uri_scheme(UriUriA *uri);
char *uri_userinfo(UriUriA *uri);
char *uri_host(UriUriA *uri);
int uri_port(UriUriA *uri);

int uri_query(UriQueryListA **dst, UriUriA *uri, int *items);
