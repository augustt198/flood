#include "file_manager.h"

#include <stdlib.h>
#include <stdbool.h>

void file_manager_init(file_manager_t *fm, torrent_t *t, char *basepath) {
    fm->torrent = t;
    fm->basepath = basepath;

    int nfiles;
    if (t->info.mode == SINGLE_FILE_MODE) {
        nfiles = 1;
    } else {
        nfiles = t->info.file_count;
    }
    fm->nfiles = nfiles;
    
    fm->descriptors = malloc(sizeof(int) * nfiles);
    for (int i = 0; i < nfiles; i++)
        fm->descriptors[i] = -1;
}

// hopefully
#define PATHBUF_SIZE 1024

void _create_file(file_manager_t *fm, int fidx) {
    char pathbuf[256];
    char *pathbuf_ptr = pathbuf;
}

void _write_piece(file_manager_t *fm, int fidx, int piecelen, char *data, int pieceoffset, int fileoffset) {
    int fd = fm->descriptors[fidx];
    if (fd == -1) {
        
    }
}

void file_manager_write_piece(file_manager_t *fm, int piece, int piecelen, char *data) {
    info_section_t info = fm->torrent->info;

    int startbyte = piece * piecelen;
    int endbyte = (piece + 1) * piecelen;
    if (info.mode == SINGLE_FILE_MODE) {
        //_write_piece(fm, piece, piecelen, data, 0);        
    } else {
        int offset = 0;
        for (int i = 0; i < fm->nfiles; i++) {
            int f_len = info.files[i].length;
            int f_start = offset, f_end = offset + f_len;
            
            // need to be very careful with off by one
            bool overlap = (f_start >= startbyte && f_start <= endbyte) ||
                            (f_end >= startbyte && f_end <= endbyte) ||
                            (f_start < startbyte && f_end > endbyte);

            if (!overlap)
                continue;

            offset += f_len; 
        }
    }
}
