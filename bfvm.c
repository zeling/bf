/*
 * Virtual machine for brainf*ck.
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
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "bf.h"

enum {
#define DEF(inst) inst,
#include "bfinst.h"
};

static int bf_bytecode_interp(uint8_t *pc, uint8_t *sp);
static int bf_default_interp(bf_t *ctx);
static size_t eat(FILE *in, char target);

void bf_init(bf_t *ctx)
{
    bf_init2(ctx, bf_default_interp);
}

void bf_init2(bf_t *ctx, int (*interp)(bf_t *))
{
    memset(ctx, 0, sizeof(bf_t));
    dynbuf_init(&ctx->bytecode);
    ctx->interp = interp;
}

int bf_load_file(bf_t *ctx, FILE *src)
{
#define EMIT(kind, expr)                                                       \
    if ((ret = dynbuf_put_##kind(bc, expr)) < 0)                               \
    goto out
#define EMITC(expr) EMIT(uint8_t, expr)
#define EMITS(expr) EMIT(size_t, expr)
    int c, ret = 0;
    dynbuf_t stack, *bc = &ctx->bytecode;
    dynbuf_init(&stack);
    do {
        switch (c = fgetc(src)) {
        case '-': {
            EMITC(DEC);
            EMITS(eat(src, '-'));
            break;
        }
        case '+': {
            EMITC(INC);
            EMITS(eat(src, '+'));
            break;
        }
        case '<': {
            EMITC(SHL);
            EMITS(eat(src, '<'));
            break;
        }
        case '>': {
            EMITC(SHR);
            EMITS(eat(src, '>'));
            break;
        }
        case '[': {
            EMITC(JZ);
            size_t here = dynbuf_size(bc);
            dynbuf_put_size_t(&stack, here);
            EMIT(ptrdiff_t, 0);
            break;
        }
        case ']': {
            EMITC(JNZ);
            size_t here = dynbuf_size(bc);
            if (dynbuf_size(&stack) < sizeof(size_t)) {
                ret = -1;
                goto out;
            }
            size_t there = dynbuf_pop_size_t(&stack);
            ptrdiff_t diff = here - there;
            EMIT(ptrdiff_t, -diff);
            memcpy(bc->data + there, &diff, sizeof(ptrdiff_t));
            break;
        }
        case '.': {
            EMITC(PUT);
            break;
        }
        case ',': {
            EMITC(GET);
            break;
        }
        default:
            /* skip */
            break;
        }
    } while (c != EOF);
    dynbuf_put_uint8_t(bc, HLT);
    assert(dynbuf_size(&stack) == 0);

out:
    dynbuf_free(&stack);
    fclose(src);
    return ret;
#undef EMITS
#undef EMITC
#undef EMIT
}

static inline void bf_unmap_tape(bf_t *ctx)
{
    size_t pgsize = getpagesize();
    assert(munmap(ctx->tape - (ctx->npage + 2) * pgsize / 2,
                  (ctx->npage + 2) * pgsize) == 0);
}

int bf_run(bf_t *ctx, size_t npage)
{
    assert(ctx->interp);
    size_t pgsize = getpagesize();
    if (ctx->tape) {
        bf_unmap_tape(ctx);
    }

    ctx->npage = npage & ~0x1;
    if (!ctx->npage)
        ctx->npage = 2;
    ctx->tape = mmap(0, (ctx->npage + 2) * pgsize, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);
    if (ctx->tape == MAP_FAILED) {
        return -1;
    }
    mprotect(ctx->tape, pgsize, PROT_NONE);
    mprotect(ctx->tape + (ctx->npage + 1) * pgsize, pgsize, PROT_NONE);
    ctx->tape = ctx->tape + (ctx->npage + 2) * pgsize / 2;
    return ctx->interp(ctx);
}

void bf_dump_bytecode(bf_t *ctx, FILE *dst)
{
    for (size_t i = 0; i < dynbuf_size(&ctx->bytecode); i++) {
        fputc(ctx->bytecode.data[i], dst);
    }
    fflush(dst);
}

void bf_free(bf_t *ctx)
{
    dynbuf_free(&ctx->bytecode);
    if (ctx->tape) {
        bf_unmap_tape(ctx);
    }
    memset(ctx, 0, sizeof(bf_t));
}

static size_t eat(FILE *in, char target)
{
    int c;
    size_t ret = 1; /* don't forget the byte initiated the process */
    while ((c = fgetc(in)) == target) {
        ++ret;
    }
    if (c != EOF) {
        ungetc(c, in);
    }
    return ret;
}

#define DEFINE_GET_T(type)                                                     \
    type get_##type(uint8_t *pc)                                               \
    {                                                                          \
        type ret;                                                              \
        memcpy(&ret, pc, sizeof(type));                                        \
        return ret;                                                            \
    }

DEFINE_GET_T(size_t)
DEFINE_GET_T(ptrdiff_t)

static int bf_bytecode_interp(uint8_t *pc, uint8_t *sp)
{
    size_t operand;
    static const void *const lut[] = {
#define DEF(inst) &&case_##inst,
#include "bfinst.h"
    };

#define SWITCH(pc) goto *lut[*pc++];
#define CASE(x) case_##x
#define BREAK SWITCH(pc)
#define RETURN(x) return x
#define OPERAND (operand = get_size_t(pc), pc += sizeof(size_t), operand)

    /* clang-format off */
    SWITCH(pc) {
    CASE(DEC):
        (*sp) -= OPERAND;
        BREAK;

    CASE(INC):
        (*sp) += OPERAND;
        BREAK;

    CASE(SHL):
        sp -= OPERAND;
        BREAK;

    CASE(SHR):
        sp += OPERAND;
        BREAK;

    CASE(JNZ):
        if (*sp)
            pc += get_ptrdiff_t(pc);
        pc += sizeof(ptrdiff_t);
        BREAK;

    CASE(JZ):
        if (!*sp)
            pc += get_ptrdiff_t(pc);
        pc += sizeof(ptrdiff_t);
        BREAK;

    CASE(PUT):
        putchar(*sp);
        BREAK;

    CASE(GET):
        *sp = getchar();
        BREAK;

    CASE(CLR):
        *sp = 0;
        BREAK;

    CASE(HLT):
        fflush(stdout);
        RETURN(0);
    }
    /* clang-format on */
}

static inline int bf_default_interp(bf_t *ctx)
{
    return bf_bytecode_interp(ctx->bytecode.data, ctx->tape);
}
