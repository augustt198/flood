#/usr/bin/env/sh
set -e

mkdir -p bin/

clang test.c bencoding.c linked_list.c -o bin/bencoding_test
./bin/bencoding_test
