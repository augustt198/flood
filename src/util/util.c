#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>

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

static pthread_mutex_t print_mutex;
static bool init_mutex = true;
int printf_safe(const char *fmt, ...) {
    if (init_mutex) {
        pthread_mutex_init(&print_mutex, NULL);
        init_mutex = false;
    }

    pthread_mutex_lock(&print_mutex);
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    pthread_mutex_unlock(&print_mutex);

    return ret;
}

void printf_lock() {
    if (init_mutex) {
        pthread_mutex_init(&print_mutex, NULL);
        init_mutex = false;
    }
    pthread_mutex_lock(&print_mutex);
}

void printf_unlock() {
    pthread_mutex_unlock(&print_mutex);
}