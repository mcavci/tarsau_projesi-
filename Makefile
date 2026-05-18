CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = tarsau

OBJ = obj/main.o obj/tarsau.o

.PHONY: all clean

all: obj $(TARGET)

obj:
	mkdir -p obj

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)


obj/main.o: src/main.c include/tarsau.h
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o


obj/tarsau.o: src/tarsau.c include/tarsau.h
	$(CC) $(CFLAGS) -c src/tarsau.c -o obj/tarsau.o

clean:
	rm -rf obj $(TARGET)