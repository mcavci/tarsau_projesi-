CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

all: bin/tarsau

bin/tarsau: lib/main.o lib/archive.o
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/tarsau lib/main.o lib/archive.o

lib/main.o: src/main.c include/tarsau.h
	@mkdir -p lib
	$(CC) $(CFLAGS) -c src/main.c -o lib/main.o

lib/archive.o: src/archive.c include/tarsau.h
	@mkdir -p lib
	$(CC) $(CFLAGS) -c src/archive.c -o lib/archive.o

clean:
	rm -rf lib/*.o bin/tarsau
