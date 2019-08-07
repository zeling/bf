/*
 * Brainf*ck interface.
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
#include <stdio.h>

#include "dynbuf.h"

typedef struct bf {
    dynbuf_t bytecode;
    uint8_t *tape;
    size_t npage;

    int (*interp)(struct bf *);
} bf_t;

void bf_init2(bf_t *ctx, int (*interp)(bf_t *));
void bf_init(bf_t *ctx);

int bf_load_file(bf_t *ctx, FILE *src);
void bf_dump_bytecode(bf_t *ctx, FILE *dst);
int bf_run(bf_t *ctx, size_t npage);

void bf_free(bf_t *ctx);
