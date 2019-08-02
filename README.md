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

