typedef struct Torrent {
    char *info_hash;
    char **trackers;
    int tracker_count;
    char *filename;
} Torrent;

int magnet2torrent(Torrent *dst, char *magnet);
int bencode2torrent(Torrent *dst, char *bencode);

void hexhash2bin(char *hash, int hlen, char *bin);
