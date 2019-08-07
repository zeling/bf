/*
 * x86-64 JIT.
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bf.h"
#include "dynbuf.h"
#include "jit.h"

/* registers:
 * - rdi: used to pass arguments,
 * - rcx: hold the pointer to sp,
 * - r14: getc,
 * - r15: putc,
 * using r14 r15 can save us one byte in indirect call */

#define REX ((uint8_t)0x40)

#define REX_W 0x8
#define REX_R 0x4
#define REX_X 0x2
#define REX_B 0x1

#define EMIT_BYTE(byte) dynbuf_put_uint8_t(&ctx->code, (uint8_t)(byte))
#define EMIT_IMM32(u32) dynbuf_put_uint32_t(&ctx->code, (uint32_t)(u32))
#define EMIT_IMM64(u64) dynbuf_put_uint64_t(&ctx->code, (uint64_t)(u64))
#define EMIT_REL32(jmp)                                                        \
    do {                                                                       \
        EMIT_BYTE(jmp);                                                        \
        size_t now = dynbuf_size(&ctx->code);                                  \
        ctx->code.size += 4;                                                   \
        return now;                                                            \
    } while (0)
#define FETCH_OP(where)                                                        \
    do {                                                                       \
        memcpy(&(where), pc, sizeof(where));                                   \
        pc += sizeof(where);                                                   \
    } while (0)
#define HERE_ABS dynbuf_size(&ctx->code)

void jit_compile_shl(jit_t *ctx, size_t arg)
{
    if (__builtin_expect(arg <= UINT32_MAX, 1)) {
        // imm32 is enough to hold arg
        EMIT_BYTE(REX | REX_W);
        EMIT_BYTE(0x81); // ADD
        EMIT_BYTE(0xE9); // reg = 5
        EMIT_IMM32(arg);
    } else {
        EMIT_BYTE(REX | REX_W);
        EMIT_BYTE(0xB8); // MOVABS rax
        EMIT_IMM64(arg);

        EMIT_BYTE(REX | REX_W);
        EMIT_BYTE(0x29);
        EMIT_BYTE(0xC1);
    }
}

void jit_compile_shr(jit_t *ctx, size_t arg)
{
    if (__builtin_expect(arg <= UINT32_MAX, 1)) {
        // imm32 is enough to hold arg
        EMIT_BYTE(REX | REX_W);
        EMIT_BYTE(0x81); // ADD
        EMIT_BYTE(0xC1);
        EMIT_IMM32(arg);
    } else {
        EMIT_BYTE(REX | REX_W);
        EMIT_BYTE(0xB8); // MOVABS rax
        EMIT_IMM64(arg);

        EMIT_BYTE(REX | REX_W);
        EMIT_BYTE(0x01); // d: 0, s: 1
        EMIT_BYTE(0xC1); // MOD: b11, REG: rax, R/M: rcx
    }
}

void jit_compile_inc(jit_t *ctx, uint8_t arg)
{
    EMIT_BYTE(0x80); // ADD
    EMIT_BYTE(0x01); // ECX
    EMIT_BYTE(arg);
}

void jit_compile_dec(jit_t *ctx, uint8_t arg)
{
    EMIT_BYTE(0x80);
    EMIT_BYTE(0x29); // MOD: 0x00, REG: 5, R/M: 1
    EMIT_BYTE(arg);
}

void jit_compile_get(jit_t *ctx)
{
    EMIT_BYTE(REX | REX_B);
    EMIT_BYTE(0xFF);
    EMIT_BYTE(0xd6);

    EMIT_BYTE(0x88);
    EMIT_BYTE(0x01);
}

void jit_compile_put(jit_t *ctx)
{
    EMIT_BYTE(REX);
    EMIT_BYTE(0x8A);
    EMIT_BYTE(0x39);

    EMIT_BYTE(REX | REX_B);
    EMIT_BYTE(0xFF);
    EMIT_BYTE(0xd7);
}

/* A loop has should have two labels:
 * - L1: start of the loop.
 * - L2: end of the loop.
 *
 * cmp BYTE PTR [rcx], 0
 * jz L2
 * L1:
 *
 * ....
 *
 * cmp BYTE PTR [rcx], 0
 * jnz L1:
 * L2:
 *
 * ...
 * */

size_t jit_compile_jnz(jit_t *ctx)
{
    EMIT_BYTE(0x80);
    EMIT_BYTE(0x39);
    EMIT_BYTE(0x00);

    EMIT_BYTE(0x0F);
    EMIT_REL32(0x85);
}

size_t jit_compile_jz(jit_t *ctx)
{
    EMIT_BYTE(0x80);
    EMIT_BYTE(0x39);
    EMIT_BYTE(0x00);

    EMIT_BYTE(0x0F);
    EMIT_REL32(0x84);
}

size_t jit_compile_ret(jit_t *ctx)
{
    EMIT_REL32(0xC3);
}

size_t jit_compile_jmp(jit_t *ctx)
{
    EMIT_REL32(0xE9);
}

void jit_patch_rel32(jit_t *ctx, size_t offset, ptrdiff_t arg)
{
    assert(arg <= 0xffffffff); /* we are limiting ourselves to near jmps */
    uint32_t rel32 = (uint32_t)arg;
    memcpy(&ctx->code.data[offset], &rel32, 4);
}

void jit_patch_abs32(jit_t *ctx, size_t offset, size_t arg)
{
    ptrdiff_t rel32 = arg - offset - 4;
    jit_patch_rel32(ctx, offset, rel32);
}

void jit_patch_ret_jmp(jit_t *ctx, size_t offset)
{
    assert(ctx->code.data[offset] == 0xC3);
    ctx->code.data[offset] = 0xE9;
}

/* Compile the loop into:
 * cmp BYTE PTR [rcx], 0
 * jz L2:
 *
 * ...
 *
 * cmp BYTE PTR [rcx], 0
 * jnz L1:
 * ret
 * [4-byte padding]
 */
size_t jit_compile_loop(jit_t *ctx, uint8_t *start, uint8_t *end)
{
    size_t operand;
    ptrdiff_t rel, fptr;
    uint32_t l1, l2;

    assert(start < end);
    assert(*start == JZ);
    assert(*end == JNZ);
    *start = CJZ;
    *end = CJNZ;

    size_t l2_ptr = jit_compile_jz(ctx);
    size_t entry = dynbuf_size(&ctx->code);
    memcpy(end + 1, &l2_ptr, sizeof(size_t));

    uint8_t *pc = start + 1 + sizeof(ptrdiff_t), *pc1;
    while (pc != end) {
        switch (*pc++) {
        case INC:
            FETCH_OP(operand);
            jit_compile_inc(ctx, operand);
            break;
        case DEC:
            FETCH_OP(operand);
            jit_compile_dec(ctx, operand);
            break;
        case SHL:
            FETCH_OP(operand);
            jit_compile_shl(ctx, operand);
            break;
        case SHR:
            FETCH_OP(operand);
            jit_compile_shr(ctx, operand);
            break;
        case GET:
            jit_compile_get(ctx);
            break;
        case PUT:
            jit_compile_put(ctx);
            break;
        case JZ:
            /* for some reason the part of the code is still not compiled */
            FETCH_OP(rel);
            pc1 = pc - 1 - sizeof(ptrdiff_t);
            jit_compile_loop(ctx, pc1, pc1 + rel);
            pc += rel;
            break;
        case JNZ:
            assert(0 && "This should be handled in JZ");
            break;
        case CJZ:
            FETCH_OP(fptr);
            pc += fptr - 1 - sizeof(ptrdiff_t);
            assert(*pc++ == CJNZ);
            FETCH_OP(fptr);
            l1 = fptr + 4;
            memcpy(&l2, ctx->code.data + fptr, 4);
            l2 += l1;
            jit_patch_abs32(ctx, jit_compile_jnz(ctx), fptr + 4);
            jit_patch_ret_jmp(ctx, l2);
            jit_patch_abs32(ctx, l2 + 1, HERE_ABS);
            break;
        case CJNZ:
            assert(0 && "This should be handled in CJZ");
            break;
        }
    }

    size_t l1_ptr = jit_compile_jnz(ctx);
    jit_patch_abs32(ctx, l1_ptr, l2_ptr + 4);
    jit_patch_abs32(ctx, l2_ptr, l1_ptr + 4);
    jit_compile_ret(ctx);
    return entry;
}
