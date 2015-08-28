#include <stdlib.h>
#include <stdio.h>

#include "bencode.h"

void die(char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void usage() {
    die("Usage: file [torrent file]\n");
}

void process_torrent_file(char *fp);

int main(int argc, char **argv) {
    if (argc < 3)
        usage();

    // torrent file path
    char *torrent_fp = argv[2];
    process_torrent_file(torrent_fp);
}

void process_torrent_file(char *fp) {
    FILE *file = fopen(fp, "rb");
    if (!file)
        die("Could not open file\n");

    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(len);
    fread(data, 1, len, file);

    bencode_value bencode;
    if (bencode_parse(data, len, &bencode) != 0)
        die("Invalid bencode\n");

    if (bencode.type != BENCODE_DICT)
        die("Expected top level value to be a dictionary\n");

    printf("Decoded torrent file:\n");
    bencode_debug(&bencode);
}
