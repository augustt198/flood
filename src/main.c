#include <stdlib.h>
#include <stdio.h>

void die(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

int main(int argc, char **argv) {
    printf("Hello, world!\n");
}
