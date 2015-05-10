typedef struct Torrent {
    char **trackers;
    int tracker_count;
    char *filename;
} Torrent;

int magnet2torrent(Torrent *dst, char *magnet);
int bencode2torrent(Torrent *dst, char *bencode);
