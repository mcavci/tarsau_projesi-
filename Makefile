CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

all: tarsau

tarsau: src/main.o lib/archive.o
	$(CC) $(CFLAGS) -o tarsau src/main.o lib/archive.o

src/main.o: src/main.c include/tarsau.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

lib/archive.o: lib/archive.c include/tarsau.h
	$(CC) $(CFLAGS) -c lib/archive.c -o lib/archive.o

clean:
	rm -f src/*.o lib/*.o tarsau