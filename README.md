#bfcc

A very stupid, educational token threaded intepreter for the very uninteresting language Brainfuck.

1. bfcc: the frontend transforms bf source code into bytecode (I won't call it a compiler anyway)
2. bfvm: the interpreter that actually executes the bytecode

./bfcc hello.bf -ohello.bfc will give you a hello.bfc in the current directory and then
./bfvm hello.bfc will run the classical helloworld example.

You can even try the generating mandelbrot sets, a good example is from [Eric Bosman](http://esoteric.sange.fi/brainfuck/utils/mandelbrot/README.txt).
If you try mandelbrot-titantic.b, you should expect the beautiful fractal image ![image](img/bf-mandelbrot-titannic.png).
At the moment, the interpreter is still slow, and I didn't find an existing interpreter that can actually run the mandelbrot program, so I can't do any comparison.
I will regard mine as the baseline and try do some optimization in the future, maybe adding some JIT to gain speed up.

Added a rpython translated rival in example2.py which is borrowed from a pypy-tutorial, with -O3 option it tooks ~32 seconds to generate a 128x48 graph while my naive
token-threaded interpreter used ~22 seconds on my box. The result may be quite satisfactory.
