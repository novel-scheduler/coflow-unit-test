# Simple makefile that loads the json-c library.

CC=gcc
CFLAGS=-I /common/home/hy368/Desktop/Research/json-c-build/ -l json-c -O2 -g -Wall
DEPS = main.h

main: main.c
	$(CC) -o $@ $^ $(CFLAGS)

playground: playground.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm main playground