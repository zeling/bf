SANITIZER = -fsanitize=address,undefined
CFLAGS = -Wall -Werror -O2 -g $(SANITIZER)
LDFLAGS = $(SANITIZER)

all: bfi quine.bf

bfi: bfi.o dynbuf.o bfvm.o
	$(CC) $(LDFLAGS) -o $@ $^ 

quine-data: quine-data.o

quine.bf: quine-code.part quine-data bfi
	./quine-data < $< > $@
	cat quine-code.part >> $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean fmt

clean:
	rm -f *.o bfi quine.bf

fmt:
	clang-format -i -style=file *.h *.c
