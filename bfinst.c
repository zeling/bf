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
    &&get
  };

  for(;;) {
    goto *lut[*pc++];
  }

dec:
  (*sp)--;
  goto *lut[*pc++];
inc:
  (*sp)++;
  goto *lut[*pc++];
shl:
  sp--;
  goto *lut[*pc++];
shr:
  sp++;
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
}
