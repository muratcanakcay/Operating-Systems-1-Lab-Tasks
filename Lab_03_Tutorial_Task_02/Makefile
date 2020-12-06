CC=gcc
CFLAGS= -std=gnu99 -Wall
LDLIBS = -lpthread -lm

all: task2

task2: task2.c
	gcc -std=gnu99 -Wall -o task2 task2.c -lpthread -lm
	
.PHONY: clean all
clean:
	rm task2