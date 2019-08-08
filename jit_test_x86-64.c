/*
 * x86-64 JIT test.
 *
 * Copyright (c) 2019 Zeling Feng
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <stdio.h>

#include "dynbuf.h"
#include "jit.h"

#define IMM(x) (uint8_t) x, 0, 0, 0, 0, 0, 0, 0

void dump_code_page(jit_t *ctx, int hex, FILE *f)
{
    assert(f);
    if (hex) {
        fwrite(ctx->code.data, 1, ctx->code.size, f);
    } else {
        for (int i = 0; i < ctx->code.size; i++) {
            fprintf(f, "%02x ", ctx->code.data[i]);
        }
    }
}

int main(int argc, char **argv)
{
    jit_t ctx;
    jit_init(&ctx);

    /* clang-format off */
    static uint8_t test_prog[] = {
        JZ,     IMM(54),
        DEC,    IMM(1),
        JZ,     IMM(18),
        DEC,    IMM(4),
        JNZ,    IMM(-18),
        DEC,    IMM(1),
        JNZ,    IMM(-54),
    };
    /* clang-format on */

    //    jit_compile_loop(&ctx, test_prog + 18, test_prog + 36);
    jit_compile_loop(&ctx, test_prog,
                     test_prog + sizeof(test_prog) - 1 - sizeof(size_t), 1);

    dump_code_page(&ctx, argc > 1, argv[1] ? fopen(argv[1], "w+") : stdout);
}
