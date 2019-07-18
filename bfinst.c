#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

uint8_t *pc;
char *sp;
uint32_t offset = 0;

void interp()
{
    int i;
    static void *lut[] = {&&dec, &&inc, &&shl, &&shr, &&jnz,
                          &&jz,  &&put, &&get, &&clr, &&exit};

    goto *lut[*pc++];

dec:
    (*sp) -= *pc++;
    goto *lut[*pc++];

inc:
    (*sp) += *pc++;
    goto *lut[*pc++];

shl:
    offset = 0;
    for (i = 0; i < 4; i++) {
        offset <<= 8;
        offset |= *pc++;
    }
    sp -= offset;
    goto *lut[*pc++];

shr:
    offset = 0;
    for (i = 0; i < 4; i++) {
        offset <<= 8;
        offset |= *pc++;
    }
    sp += offset;
    goto *lut[*pc++];

jnz:
    offset = 0;
    for (i = 0; i < 4; i++) {
        offset <<= 8;
        offset |= *pc++;
    }
    if (*sp)
        pc += (int32_t)offset;
    goto *lut[*pc++];

jz:
    offset = 0;
    for (i = 0; i < 4; i++) {
        offset <<= 8;
        offset |= *pc++;
    }
    if (!*sp)
        pc += (int32_t)offset;
    goto *lut[*pc++];

put:
    putchar(*sp);
    goto *lut[*pc++];

get:
    *sp = getchar();
    goto *lut[*pc++];

clr:
    *sp = 0;
    goto *lut[*pc++];

exit:
    exit(0);
}
