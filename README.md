# flood

A BitTorrent client.

**Capabilities**
- Listing peers via UDP trackers

Example using magnet link:
```
./bin/out magnet "magnet:?xt=urn:btih:b415c913643e5ff49fe37d304bbb5e6e11ad5101&dn=Ubuntu+14.10+desktop++x64&tr=udp%3A%2F%2Ftracker.openbittorrent.com%3A80&tr=udp%3A%2F%2Fopen.demonii.com%3A1337&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Fexodus.desync.com%3A6969"
request transaction id: 1334752205
response transaction id: 1334752205
connection id: -7151309174462965319
Peers (82):
27.255.196.61:44522
41.138.100.139:30302
41.221.101.182:45682
50.205.187.142:6881
...
```

### Installation

- Install Doxygen & Graphviz (needed by uriparser)
- `sh build.sh`

### Todo

- Handling HTTP trackers
- Downloading from peers
- Other stuff a BitTorrent client would do

[![travis badge](https://api.travis-ci.org/augustt198/flood.svg)](https://travis-ci.org/augustt198/flood)
