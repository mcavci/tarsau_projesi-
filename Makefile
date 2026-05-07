all: tarsau

tarsau: main.c
	gcc -Wall -o tarsau main.c

clean:
	rm -f tarsau