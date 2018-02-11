#pragma once

#include "torrent.h"

typedef struct file_manager {
    torrent_t *torrent;

    char *basepath;
    
    // array of file descriptors
    int nfiles;
    int *descriptors;
} file_manager_t;

void file_manager_init(file_manager_t *f, torrent_t *t, char *basepath);

// writes a piece
void file_manager_write_piece(file_manager_t *f, int piece, int piecelen, char *piecedata);
