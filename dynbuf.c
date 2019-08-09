/*
 * Dynamic buffer implementation.
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
#include <stdlib.h>
#include <string.h>

#include "dynbuf.h"

static int dynbuf_default_realloc(dynbuf_t *, size_t);

void dynbuf_init(dynbuf_t *buf)
{
    dynbuf_init1(buf, &dynbuf_default_realloc);
}

void dynbuf_init1(dynbuf_t *buf, dynbuf_realloc_t realloc)
{
    memset(buf, 0, sizeof(dynbuf_t));
    buf->data = NULL;
    buf->cap = 0;
    buf->size = 0;
    buf->realloc = realloc;
}

int dynbuf_realloc(dynbuf_t *buf, size_t new_size)
{
    return buf->realloc(buf, new_size);
}

int dynbuf_default_realloc(dynbuf_t *buf, size_t new_size)
{
    size_t cap = buf->cap;
    if (cap >= new_size && new_size >= cap / 2) {
        return 0;
    }

    if (!new_size) {
        if (buf->data)
            free(buf->data);
        buf->data = NULL;
        buf->cap = 0;
        return 0;
    }

    new_size = new_size + (new_size >> 3) + (new_size < 9 ? 3 : 6);
    if (!(buf->data = realloc(buf->data, new_size))) {
        return -1;
    }

    buf->cap = new_size;
    return 0;
}

int dynbuf_put(dynbuf_t *buf, const uint8_t *ptr, size_t count)
{
    size_t size = dynbuf_size(buf);
    if (dynbuf_realloc(buf, size + count) < 0) {
        return -1;
    }
    memcpy(buf->data + size, ptr, count);
    buf->size += count;
    return 0;
}

size_t inline dynbuf_size(dynbuf_t *buf)
{
    return buf->size;
}

void dynbuf_free(dynbuf_t *buf)
{
    if (buf->data)
        buf->realloc(buf, 0);
    memset(buf, 0, sizeof(dynbuf_t));
}

#define DYNBUF_DEFINE_PUT_T(type)                                              \
    int dynbuf_put_##type(dynbuf_t *buf, type arg)                             \
    {                                                                          \
        return dynbuf_put(buf, (uint8_t *)&arg, sizeof(type));                 \
    }

#define DYNBUF_DEFINE_POP_T(type)                                              \
    type dynbuf_pop_##type(dynbuf_t *buf)                                      \
    {                                                                          \
        size_t size = dynbuf_size(buf);                                        \
        assert(size >= sizeof(type));                                          \
        size -= sizeof(type);                                                  \
        type ret;                                                              \
        memcpy(&ret, buf->data + size, sizeof(type));                          \
        dynbuf_realloc(buf, size);                                             \
        buf->size = size;                                                      \
        return ret;                                                            \
    }

#define DEFINE_DYNBUF(type)                                                    \
    DYNBUF_DEFINE_PUT_T(type)                                                  \
    DYNBUF_DEFINE_POP_T(type)

DYNBUF_ELEMENT_LIST(DEFINE_DYNBUF)

#undef DEFINE_DYNBUF
