CC=clang
CFLAGS=-Wall -g -L/usr/local/lib -luriparser
LD=clang++

INCLUDES=-Iincludes/ -Iincludes/util/
SOURCES=$(wildcard src/*.c) $(wildcard src/util/*.c)

all: ${SOURCES}
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o bin/flood

clean:
	@rm bin/flood
