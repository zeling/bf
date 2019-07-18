CC=gcc
CFLAGS = -Wall -Werror -O2 -g $(U)

all: bfi

bfi: bfi.o bfvm.o bfinst.o bfcc.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c $< $(CFLAGS)

.phony clean:
	rm *.o bfi
