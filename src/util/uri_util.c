#include "uri_util.h"

int parse_uri(UriUriA *dst, char *str) {
    UriParserStateA state;
    state.uri = dst;

    return uriParseUriA(&state, str);
}

char *textrange2str(UriTextRangeA *range) {
    return strndup(
        range->first,
        range->afterLast - range->first
    );
}

char *uri_scheme(UriUriA *uri) {
    return textrange2str(&(uri->scheme));
}

char *uri_userinfo(UriUriA *uri) {
    return textrange2str(&(uri->userInfo));
}

char *uri_host(UriUriA *uri) {
    return textrange2str(&(uri->hostText));
}

int uri_port(UriUriA *uri) {
    UriTextRangeA range = uri->portText;
    int port = 0;

    for (const char *p = range.first; p < range.afterLast; p++)
        port = (port * 10) + (*p - '0');

    return port;
}

int uri_query(UriQueryListA **dst, UriUriA *uri, int *items) {
    return uriDissectQueryMallocA(
        dst, items,
        uri->query.first, uri->query.afterLast
    );
}
