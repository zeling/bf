SANITIZER = -fsanitize=address,undefined
CFLAGS = -Wall -Werror -O2 -g $(SANITIZER) -D_GNU_SOURCE
LDFLAGS = $(SANITIZER) -lc

all: bfi test quine.bf
.PHONY: clean fmt

test: jit_test_x86-64.c dynbuf.o bfvm.o jit_entry_x86-64.o jit.o jit_x86-64.o
	$(CC) $(LDFLAGS) -o $@ $^

bfi: bfi.o dynbuf.o bfvm.o jit.o jit_entry_x86-64.o jit_x86-64.o
	$(CC) $(LDFLAGS) -o $@ $^

quine.bf: quine-code.part quine-data.bf bfi
	./bfi quine-data.bf < $< > $@
	cat quine-code.part >> $@

fmt:
	clang-format -i -style=file *.h *.c

clean:
	rm -f *.o bfi quine.bf

