#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef DEBUG
#define LABEL(name) name: printf("sp: %p, pc: %p, char: %c, inst: %s\n", sp, pc, *sp, #name);
#else
#define LABEL(name) name:
#endif

uint8_t *pc;
char *sp;
uint8_t *dumb = 0;
uint32_t offset = 0;
uint8_t **place;
int vardec = 0;

void interp() {
  int i;
  static void *lut[] = {
    &&dec,
    &&inc,
    &&shl,
    &&shr,
    &&jnz,
    &&jz,
    &&put,
    &&get,
    &&clr,
    &&exit
  };

  goto *lut[*pc++];

LABEL(dec)
  (*sp) -= *pc++;
  goto *lut[*pc++];
LABEL(inc)
  (*sp) += *pc++;
  goto *lut[*pc++];
LABEL(shl)
  offset = 0;
  for (i = 0; i < 4; i++) {
    offset <<= 8;
    offset |= *pc++;
  }
  sp -= offset;
  goto *lut[*pc++];
LABEL(shr)
  offset = 0;
  for (i = 0; i < 4; i++) {
    offset <<= 8;
    offset |= *pc++;
  }
  sp += offset;
  goto *lut[*pc++];
LABEL(jnz)
  offset = 0;
  for (i = 0; i < 4; i++) {
    offset <<= 8;
    offset |= *pc++;
  }
  if (*sp) pc += (int32_t) offset;
  goto *lut[*pc++];
LABEL(jz)
  offset = 0;
  for (i = 0; i < 4; i++) {
    offset <<= 8;
    offset |= *pc++;
  }
  if (!*sp) pc += (int32_t) offset;
  goto *lut[*pc++];
LABEL(put)
  putchar(*sp);
  goto *lut[*pc++];
LABEL(get)
  *sp = getchar();
  goto *lut[*pc++];
LABEL(clr)
  *sp = 0;
  goto *lut[*pc++];
LABEL(exit)
  exit(0);
  
}
