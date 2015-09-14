#pragma once

typedef struct file_info {
    int  file_count;
    char **files;
    
} file_info_t;

typedef struct torrent {
    char        info_hash[20];
    file_info_t info;
    
    int  tracker_count;
    char **trackers;

    char *download_name;
} torrent_t;

int torrent_init_from_file(char *filepath, torrent_t *t);

int torrent_init_from_magnet(char *magnet);
