CC=gcc
CFLAGS = -O2

bfcc: bfcc.o
	$(CC) -o bfcc bfcc.o

bfcc.o: bfcc.c
	$(CC) -c bfcc.c $(CFLAGS)

bfinst.o: bfinst.c
	$(CC) -c bfinst.c $(CFLAGS)


