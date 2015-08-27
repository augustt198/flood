#include "discover_ip.h"

#include <curl/curl.h>

size_t write_fn(char *ptr, size_t size, size_t nmemb, void *data) {
    *((char**) data) = ptr;

    return size * nmemb;
}

int discover_ip_str(char **strp) {
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, "http://api.ipify.org/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, strp);
    CURLcode code = curl_easy_perform(curl);

    return code;
}

int discover_ip(struct in_addr *ip) {
    char *data;

    int code = discover_ip_str(&data);
    if (code != CURLE_OK)
        return code;

    // inet_aton returns 1 on success,
    // we want 0 on success 
    return !inet_aton(data, ip);
}
