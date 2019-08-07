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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#include "dynbuf.h"
#include "jit.h"

#define container_of(ptr, type, member)                                        \
    (type *)((char *)(ptr)-offsetof(type, member))

static int dynbuf_mmap_realloc(dynbuf_t *, size_t);

void jit_init(jit_t *ctx)
{
    dynbuf_init1(&ctx->code, &dynbuf_mmap_realloc);
    ctx->npage = 0;
}

int dynbuf_mmap_realloc(dynbuf_t *buf, size_t new_size)
{
    jit_t *ctx = container_of(buf, jit_t, code);
    size_t pgsize = getpagesize();
#if defined(__linux__)
    size_t new_npage = (new_size + pgsize - 1) / pgsize;

    size_t old_size = ctx->npage * pgsize;
    new_size = new_npage * pgsize;

    if (old_size == new_size)
        return 0;

    if (!old_size && new_size) {
        buf->data = mmap(0, new_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON - 1, 0);
        if (!buf->data)
            return -1;
    } else if (old_size && !new_size) {
        munmap(buf->data, old_size);
    } else if (old_size && new_size) {
        assert(buf->data);
        mremap(buf->data, old_size, new_size, 0);
    }
#elif defined(__APPLE__)
    // lets do some silly things now:
    if (!buf->data) {
        buf->data = mmap(0, pgsize * 16, PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANON, -1, 0);
        assert(buf->data != MAP_FAILED);
        if (buf->data == MAP_FAILED)
            return -1;
        ctx->npage = 16;
    }
#else
#error not supported
#endif
    return 0;
}

void jit_make_writable(jit_t *ctx)
{
    size_t pgsize = getpagesize();
    mprotect(ctx->code.data, ctx->npage * pgsize, PROT_READ | PROT_WRITE);
}

void jit_make_executable(jit_t *ctx)
{
    size_t pgsize = getpagesize();
    mprotect(ctx->code.data, ctx->npage * pgsize, PROT_READ | PROT_EXEC);
}

extern void jit_entry(uintptr_t sp, uintptr_t pc);

void jit_enter(jit_t *ctx, uint8_t *sp)
{
    uintptr_t pc = ((uintptr_t)ctx->code.data) + ctx->entry;
    jit_entry((uintptr_t)sp, pc);
}

ptrdiff_t get_ptrdiff_t(uint8_t *pc);
size_t get_size_t(uint8_t *pc);

static int bf_jit_interp(bf_t *bf)
{
    uint8_t *pc = bf->bytecode.data, *pc1;
    uint8_t *sp = bf->tape;
    size_t operand;
    jit_t ctx;
    jit_init(&ctx);

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
            pc1 = pc + get_ptrdiff_t(pc) - 1;
            if (*sp) {
                jit_make_writable(&ctx);
                ctx.entry = jit_compile_loop(&ctx, pc1, pc - 1);
                jit_make_executable(&ctx);
                jit_enter(&ctx, sp);
            }
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
            goto out;
    }
    /* clang-format on */

out:
    fflush(stdout);
    jit_free(&ctx);
    return 0;
}

void bf_init_jit(bf_t *bf)
{
    bf_init2(bf, bf_jit_interp);
}

void jit_free(jit_t *ctx)
{
}
