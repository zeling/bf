#include <stdio.h>
#include <stdlib.h>

enum bfinst {
  I_DEC = 0,
  I_INC,
  I_SHL,
  I_SHR,
  I_JNZ,
  I_JZ,
  I_PUT,
  I_GET
};

typedef struct stack {
  size_t cap;
  size_t top;
  long *data;
} stack_t;

stack_t stack_new() {
#define DEFAULT_SIZE 64
  long *data = malloc(sizeof(long) * DEFAULT_SIZE);
  return {
    .cap = DEFAULT_SIZE;
    .top = 0;
    .data = data;
  }
}

void push(stack_t stack, long pos) {
  if (stack.top >= stack.cap) {
    stack.cap *= 2;
    stack.data = realloc(stack.data, stack.cap);
  } 
  stack.data[stack.top++] = pos;
}

long pop(stack_t stack) {
  int ret = stack.data[--stack.top];
  if (stack.top <= stack.cap / 4) {
    stack.cap /= 2;
    stack.data = realloc(stack.data, stack.cap);
  }
  return ret;
}

void transform(FILE *in, FILE *out) {
  int c;
  stack_t st = stack_new();
  do {
    switch (c = fgetc(in)) {
      case '-': {
        fputc(I_DEC, out);
	break;
      }
      case '+': {
        fputc(I_INC, out);
	break;
      }
      case '<': {
        fputc(I_SHL, out);
	break;
      }
      case '>': {
        fputc(I_SHR, out);
	break;
      }
      case '[': {
        fputc(I_JZ, out);
	long pos = ftell(out);
	push(st, pos);
	fputc(0, out); /* we wiil come back later */
	break;
      }
      case ']': {
        fputc(I_JNZ, out);
	break;
      }
      case '.': {
        fputc(I_PUT, out);
	break;
      }
      case ',': {
        fputc(I_GET, out);
	break;
      }
    }
  } while (c != EOF);
}


