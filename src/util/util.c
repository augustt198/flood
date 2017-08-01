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

// peer ID analysis

static const char *peer_client_lookup[] = {
    "7T", "aTorrent Android",
    "AB", "AnyAvent::BitTorrent",
    "AG", "Ares",
    "A~", "Ares",
    "AR", "Arctic",
    "AV", "Avicora",
    "AT", "Artemis",
    "AX", "BitPump",
    "AZ", "Azureus",
    "BB", "BitBuddy",
    "BC", "BitComet",
    "BE", "Baretorrent",
    "BF", "Bitflu",
    "BG", "BTG",
    "BL", "BitCometLite",
    "BL", "BitBlinder",
    "BP", "BitTorrent Pro",
    "BR", "BitRocket",
    "BS", "BRSlave",
    "BT", "mainline BitTorrent",
    "Bt", "Bt",
    "BX", "~Bittorrent X",
    "CD", "Enhanced CTorrent",
    "CT", "CTorrent",
    "DE", "DelugeTorrent",
    "DP", "Propagate Data Client",
    "EB", "EBit",
    "ES", "electric sheep",
    "FC", "FileCroc",
    "FD", "Free Download Manager",
    "FT", "FoxTorrent",
    "FX", "Freebox BitTorrent",
    "GS", "GSTorrent",
    "HK", "Hekate",
    "HL", "Halite",
    "HM", "hMule",
    "HN", "Hydranode",
    "IL", "iLivid",
    "JS", "Justseed.it client",
    "JT", "JavaTorrent",
    "KG", "KGet",
    "KT", "KTorrent",
    "LC", "LeechCraft",
    "LH", "LH-ABC",
    "LP", "Lphant",
    "LT", "libtorrent",
    "lt", "libtorrent",
    "LW", "LimeWire",
    "MK", "Meerkat",
    "MO", "MonoTorrent",
    "MP", "MooPolice",
    "MR", "Miro",
    "MT", "MoonlightTorrent",
    "NB", "Net::BitTorrent",
    "NX", "Net Transport",
    "OS", "OneSwarm",
    "OT", "OmegaTorrent",
    "PB", "Protocol::BitTorrent",
    "PD", "Pando",
    "PI", "PicoTorrent",
    "PT", "PHPTracker",
    "qB", "qBittorrent",
    "QD", "QQDownload",
    "QT", "Qt 4 Torrent example",
    "RT", "Retriever",
    "RZ", "RezTorrent",
    "S~", "Shareaza",
    "SB", "~Swiftbit",
    "SD", "Thunder",
    "SM", "SoMud",
    "SP", "BitSpirit",
    "SS", "SwarmScope",
    "ST", "SymTorrent",
    "st", "sharktorrent",
    "SZ", "Shareaza",
    "TB", "Torch",
    "TE", "terasaur Seed Bank",
    "TL", "Tribler",
    "TN", "TorrentDotNET",
    "TR", "Transmission",
    "TS", "Torrentstorm",
    "TT", "TuoTu",
    "UL", "uLeecher",
    "UM", "uTorrent Mac",
    "UT", "uTorrent",
    "VG", "Vagaa",
    "WD", "WebTorrent Desktop",
    "WT", "BitLet",
    "WW", "WebTorrent",
    "WY", "FireTorrent",
    "XF", "Xfplay",
    "XL", "Xunlei",
    "XS", "XSwifter",
    "XT", "XanTorrent",
    "XX", "Xtorrent",
    "ZT", "ZipTorrent",
    NULL, NULL
};

int _is_dig(char c) {
    return c >= '0' && c <= '9';
}

int peer_id_analysis(const char *id, const char **client_name, int *major, int *minor, int *patch) {
    if (id[0] != '-' || id[7] != '-')
        return -1;

    if (!(_is_dig(id[3]) && _is_dig(id[4]) && _is_dig(id[5]) && _is_dig(id[6])))
        return -1;
    
    int found = 0;
    for (int i = 0; peer_client_lookup[i] != NULL; i += 2) {
        const char *shortname = peer_client_lookup[i];
        const char *longname  = peer_client_lookup[i+1];
        if (shortname[0] == id[1] && shortname[1] == id[2]) {
            *client_name = longname;
            found = 1;
        }
    }
    if (found == 0)
        return -1;

    *major = id[3] - '0';
    *minor = id[4] - '0';
    *patch = id[5] - '0';
    return 0;
}

int peer_id_friendly(const char *id, char **out) {
    const char *client;
    int major, minor, patch;

    int res = peer_id_analysis(id, &client, &major, &minor, &patch);
    if (res != 0)
        return res;
    
    asprintf(out, "%s %d.%d.%d", client, major, minor, patch);
    return 0;
}
