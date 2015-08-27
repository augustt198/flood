#pragma once

#include <arpa/inet.h>

// utility for finding our public IP,
// using https://www.ipify.org/

// Place a string representation of
// the IP at strp
//
// returns 0 on success
int discover_ip_str(char **strp);

// Place the IP into the given in_addr
//
// returns 0 on success
int discover_ip(struct in_addr *addrp);
