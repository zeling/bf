CC=gcc
CFLAGS = -Wall -Werror -O2 -g $(U)

all: bfcc bfvm

bfcc: bfcc.o
	$(CC) -o bfcc bfcc.o

bfcc.o: bfcc.c
	$(CC) -c bfcc.c $(CFLAGS)

bfinst.o: bfinst.c
	$(CC) -c bfinst.c $(CFLAGS)

bfvm.o: bfvm.c
	$(CC) -c bfvm.c $(CFLAGS)

bfvm: bfvm.o bfinst.o
	$(CC) -o bfvm bfvm.o bfinst.o

.phony clean:
	rm *.o bfvm bfcc *.bfc
