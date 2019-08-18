# Brainf*ck

## Build
It should be fairly easy for anyone who has `make` installed.

## The interpreter
run `./bfi <bf source>`
or simply omit the argument and provide your source using the stdin.
```
./bfi <<EOF
++++++++++
[>+++++++>
++++++++++
>+++>+<<<<
-]>++.>+.+
++++++..++
+.>++.<<++
++++++++++
+++.>.+++.
------.---
-----.>+.>.
EOF
```
should give you back the classic "Hello World!"


## A section on quine

A quine is included, and its construction is also included.

Specifically, You can take a look at the `Makefile`, `quine-data.c`
and `quine-code.part`. These three files should give you some clue
about how the quine is constructed.

To generate it by yourself, you can first remove `quine.bf` by `rm quine.bf`
and then run `make quine.bf`, or simply `make clean quine.bf`.

To check that it is indeed a quine, `diff` should be helpful.
`diff quine.bf <(./bfi quine.bf)` should show you nothing.


## JIT
This BF interpreter also has JIT support on x86-64. The JIT will
kick in every time a real backward jump happens, i.e., `]` encountered
when the current cell is non-zero. This avoids compilation when the
loop is never or only executed once. When the loop is finished, the
execution will be handled back to the interpreter and the interpreter
will start executing bytecode until the next backward jump happens.
The assembler is currently hacky but it works. The blocks are organized
in the code page and are linked by jumps.

As measured, the performance boost of the JIT compiler on a Macbook Pro
2018 is 10x on mandelbrot-titanic.b.

