#pragma once

typedef struct file_info {
    char *file_name;
    int  length;
    int path_segments;
    char **path;
} file_info_t;

typedef struct info_section_t {
    int piece_length;
    int pieces_bytes;
    char *pieces;

    enum { SINGLE_FILE_MODE, MULTI_FILE_MODE } mode;
    char *file_name;
    union {
        struct {
            int file_length;
        };
        struct {
            int file_count;
            file_info_t *files;
        };
    };
} info_section_t;

typedef struct torrent {
    char info_hash[20];
    info_section_t info;

    int  tracker_count;
    char **tracker_urls;
} torrent_t;

int torrent_init_from_file(char *filepath, torrent_t *t);

int torrent_init_from_magnet(char *magnet);

void print_info_section(info_section_t *info);
