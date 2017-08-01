#pragma once

#include <stdio.h>

#ifdef __FLOOD_DEBUG
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...)
#endif

long read_file(FILE *file, char **dst);

int printf_safe(const char *fmt, ...);
void printf_lock();
void printf_unlock();

// extract client name and version from a peer ID
int peer_id_analysis(const char *id, const char **client_name, int *major, int *minor, int *patch);
// get a user friendly name from a peer ID
int peer_id_friendly(const char *id, char **out);