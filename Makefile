CC=gcc
CFLAGS = -Wall -Werror -O2 -g -fsanitize=address
LDFLAGS = -fsanitize=address

all: bfi

bfi: bfi.o dynbuf.o bfvm.o
	$(CC) $(LDFLAGS) -o $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean fmt

clean:
	rm *.o bfi

fmt:
	clang-format -i -style=file *.h *.c
