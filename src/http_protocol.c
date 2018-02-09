#include "http_protocol.h"

#include "bencode.h"

#include <curl/curl.h>

// how much buffer need to be multiplied by for
// escape characters to fit
#define ESCAPE_MULT 6

struct curl_memory_buf {
    char *data;
    size_t size;
};

size_t curl_write_memory_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct curl_memory_buf *mem = (struct curl_memory_buf*) userp;
    
    // grow data
    mem->data = realloc(mem->data, mem->size + realsize + 1);

    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0; // 0 terminate

    return realsize;
}

int http_tracker_request(uri_t uri, http_tracker_request_t *req, http_tracker_response_t *res) {

    uri_query_list_t *query = NULL;
    uri_query_list_t *base;

    // build query string 
    char info_hash_escaped[20*ESCAPE_MULT] = {0};
    uriEscapeA(req->info_hash, info_hash_escaped, URI_TRUE, URI_FALSE);
    query = uri_query_list_append("info_hash", info_hash_escaped, query);
    base = query;

    char peer_id_escaped[20*ESCAPE_MULT] = {0};
    uriEscapeA(req->peer_id, peer_id_escaped, URI_TRUE, URI_FALSE);
    query = uri_query_list_append("peer_id", peer_id_escaped, query);

    char *port_string;
    asprintf(&port_string, "%d", req->port);
    query = uri_query_list_append("port", port_string, query);

    char *uploaded_string;
    asprintf(&uploaded_string, "%d", req->uploaded);
    query = uri_query_list_append("uploaded", uploaded_string, query);

    char *downloaded_string;
    asprintf(&downloaded_string, "%d", req->downloaded);
    query = uri_query_list_append("downloaded", downloaded_string, query);

    char *left_string;
    asprintf(&left_string, "%d", req->left);
    query = uri_query_list_append("left", left_string, query);

    char *compact_string = req->compact ? "1" : "0";
    query = uri_query_list_append("compact", compact_string, query);

    char *no_peer_id_string = req->no_peer_id ? "1" : "0";
    query = uri_query_list_append("no_peer_id", no_peer_id_string, query);

    if (req->ip != NULL) {
        query = uri_query_list_append("ip", req->ip, query);
    }

    char *numwant_string;
    asprintf(&numwant_string, "%d", req->numwant);
    query = uri_query_list_append("numwant", numwant_string, query);

    if (req->key != NULL) {
        query = uri_query_list_append("key", req->key, query);
    }

    if (req->trackerid != NULL) {
        query = uri_query_list_append("trackerid", req->trackerid, query);
    }

    char *query_str;
    uriComposeQueryMallocA(&query_str, base);

    uri.query = str2textrange(query_str);

    char *req_url;
    int rescode = uri_to_string(&uri, &req_url);

    struct curl_memory_buf curlbuf;
    curlbuf.data = malloc(1);
    curlbuf.size = 0;

    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, req_url);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_memory_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &curlbuf);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
   
    CURLcode curl_rescode = curl_easy_perform(curl);

    if (curl_rescode == CURLE_OK) {
        printf("CURL http request completed succesfully\n");
        printf("Some of the string: %.30s\n", curlbuf.data);
        return;
    } else {
        printf("CURL error: %s\n", curl_easy_strerror(curl_rescode));
    }

    bencode_value bval;
    int parsecode = bencode_parse(curlbuf.data, curlbuf.size, &bval);
    if (parsecode == 0) {
        printf(">>>>>>>>> SUCCESSFULLY PARSED BENCODE\n");
    } else {
        printf(">>>>>>>>> COULDN'T PARSE BENCODE\n");
    }

    return 0;
}
