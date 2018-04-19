#include <stdint.h>

uint8_t *pc;
char *sp;

void interp() {
  static void *lut[] = {
    &&dec,
    &&inc,
    &&shl,
    &&shr,
    &&jnz,
    &&jz,
    &&put,
    &&get,
    &&clr
  };

  for(;;) {
    goto *lut[*pc++];
  }

dec:
  (*sp) -= *pc++;
  goto *lut[*pc++];
inc:
  (*sp) += *pc++;
  goto *lut[*pc++];
shl:
  sp -= *pc++;
  goto *lut[*pc++];
shr:
  sp += *pc++;
  goto *lut[*pc++];
jnz:
  if (*sp) pc += (signed) (*pc);
  ++pc;
  goto *lut[*pc++];
jz:
  if (!*sp) pc += (signed) (*pc);
  ++pc;
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
}
