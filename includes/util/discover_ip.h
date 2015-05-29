#include <arpa/inet.h>
#include <curl/curl.h>

int discover_ip_str(char **strp);
int discover_ip(struct in_addr *addrp);
