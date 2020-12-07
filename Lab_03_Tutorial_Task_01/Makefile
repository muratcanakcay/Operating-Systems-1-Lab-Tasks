CC=gcc
CFLAGS= -std=gnu99 -Wall
LDLIBS = -lpthread -lm
all: task1_stage1 task1_stage2 task1_stage3 task1_stage4

task1_stage1: task1_stage1.c
	gcc -o task1_stage1 task1_stage1.c -lpthread -lm

task1_stage2: task1_stage2.c
	gcc -o task1_stage2 task1_stage2.c -lpthread -lm

task1_stage3: task1_stage3.c
	gcc -o task1_stage3 task1_stage3.c -lpthread -lm

task1_stage4: task1_stage4.c
	gcc -o task1_stage4 task1_stage4.c -lpthread -lm
	
.PHONY: clean all
clean:
	rm task1_stage1 task1_stage2 task1_stage3 task1_stage4