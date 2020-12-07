CC=gcc
CFLAGS= -std=gnu99 -Wall
LDLIBS = -lpthread -lm

all: pct_stage1 pct_stage2 pct_stage3 pct_stage4
pct_stage1: pct_stage1.c
	gcc -o pct_stage1 pct_stage1.c -lpthread -lm
pct_stage2: pct_stage2.c
	gcc -o pct_stage2 pct_stage2.c -lpthread -lm
pct_stage3: pct_stage3.c
	gcc -o pct_stage3 pct_stage3.c -lpthread -lm
pct_stage4: pct_stage4.c
	gcc -o pct_stage4 pct_stage4.c -lpthread -lm
	
.PHONY: clean all
clean:
	rm pct_stage1 pct_stage2 pct_stage3 pct_stage4