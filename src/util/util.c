#include "util.h"

#include <stdlib.h>

long read_file(FILE *file, char **dst) {
    if (fseek(file, 0, SEEK_END) != 0)
        return -1;  

    long len = ftell(file);

    if (fseek(file, 0, SEEK_SET) != 0)
        return -1;

    *dst = malloc(len);
    fread(*dst, 1, len, file);

    return len;
}
