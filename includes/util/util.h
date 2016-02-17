#pragma once

#include <stdio.h>

#ifdef __FLOOD_DEBUG
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...)
#endif

long read_file(FILE *file, char **dst);
