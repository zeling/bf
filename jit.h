/*
 * Common JIT interface.
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
#pragma once
#include "bf.h"
#include "dynbuf.h"

enum {
#define DEF(inst) inst,
#include "bfinst.h"
    CJZ,  /* the following ptrdiff_t is a forwarding pointer */
    CJNZ, /* the following ptrdiff_t is a forwarding pointer */
};

typedef struct jit {
    dynbuf_t code;
    /* total pages currently mapped */
    int npage;
    size_t entry;
} jit_t;

void jit_init(jit_t *ctx);
void jit_free(jit_t *ctx);

void jit_make_writable(jit_t *ctx);
void jit_make_executable(jit_t *ctx);
void jit_enter(jit_t *ctx, uint8_t *sp);

size_t jit_compile_loop(jit_t *ctx, uint8_t *start, uint8_t *end);
void bf_init_jit(bf_t *bf);
