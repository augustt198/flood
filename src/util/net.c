#include "net.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "uri_util.h"

int create_udp_socket(char *url, int *fd, struct sockaddr **res_addr) {
    uri_t uri;
    
    int res = parse_uri(&uri, url);
    if (res != 0) {
        return res;
    }

    struct addrinfo *addr;
    res = getaddrinfo(uri_host(&uri), NULL, NULL, &addr);
    if (res != 0) {
        return res;
    }

    struct sockaddr_in *sock_addr = (struct sockaddr_in*) addr->ai_addr;
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port   = htons(uri_port(&uri));

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        return -1;
    }
    *fd = sock;
    *res_addr = addr->ai_addr;
    return 0;
}
