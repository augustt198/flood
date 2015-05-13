CC=clang
CFLAGS=-Wall -g
LD=clang++
SOURCES=$(wildcard *.c) $(URI_PARSER)

all: ${SOURCES}
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SOURCES) -o bin/out
