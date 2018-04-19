CC=gcc
CFLAGS = -O2

bfinst.o: bfinst.c
	$(CC) -c bfinst.c $(CFLAGS)
