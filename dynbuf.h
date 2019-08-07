/*
 * Dynamic buffer API.
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
#include <stddef.h>
#include <stdint.h>

typedef struct dynbuf {
    size_t cap;
    size_t size;
    uint8_t *data;
    int (*realloc)(struct dynbuf *, size_t);
} dynbuf_t;

typedef int (*dynbuf_realloc_t)(dynbuf_t *, size_t);

void dynbuf_init(dynbuf_t *buf);
void dynbuf_init1(dynbuf_t *buf, dynbuf_realloc_t realloc);
int dynbuf_realloc(dynbuf_t *buf, size_t new_size);
int dynbuf_put(dynbuf_t *buf, const uint8_t *data, size_t count);
size_t dynbuf_size(dynbuf_t *buf);
void dynbuf_free(dynbuf_t *buf);

#define DYNBUF_ELEMENT_LIST(T)                                                 \
    T(uint8_t)                                                                 \
    T(size_t)                                                                  \
    T(ptrdiff_t)                                                               \
    T(uint64_t)                                                                \
    T(uint32_t)

#define DECLARE_DYNBUF(type)                                                   \
    int dynbuf_put_##type(dynbuf_t *, type);                                   \
    type dynbuf_pop_##type(dynbuf_t *);

DYNBUF_ELEMENT_LIST(DECLARE_DYNBUF)

#undef DECLARE_DYNBUF
