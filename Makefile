CC=clang
CFLAGS=-Wall -g -luriparser -lpthread -lcurl
LD=clang++

INCLUDES=-Iincludes/ -Iincludes/util/
SOURCES=$(wildcard src/*.c) $(wildcard src/util/*.c)

MAIN_FILE=src/main.c
SOURCES:=$(filter-out $(MAIN_FILE), $(SOURCES))

all: $(SOURCES) $(MAIN_FILE)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN_FILE) $(SOURCES) -o bin/flood

test: $(SOURCES) test/bencode_test.c
	@mkdir -p test/bin
	$(CC) $(CFLAGS) $(INCLUDES) test/bencode_test.c $(SOURCES) -o bin/bencode_test

clean:
	@rm bin/flood
