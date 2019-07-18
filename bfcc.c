#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum bfinst {
  I_DEC = 0,
  I_INC,
  I_SHL,
  I_SHR,
  I_JNZ,
  I_JZ,
  I_PUT,
  I_GET,
  I_CLR,
  I_EXIT
};

typedef struct stack {
  size_t cap;
  size_t top;
  long *data;
} stack_t;

stack_t stack_new() {
#define DEFAULT_SIZE 64
  long *data = malloc(sizeof(long) * DEFAULT_SIZE);
  stack_t ret = {.cap = DEFAULT_SIZE, .top = 0, .data = data};
  return ret;
}

void push(stack_t *stack, long pos) {
  if (stack->top >= stack->cap) {
    stack->cap *= 2;
    stack->data = realloc(stack->data, stack->cap * sizeof(long));
  }
  stack->data[stack->top++] = pos;
}

long pop(stack_t *stack) {
  int ret = stack->data[--stack->top];
  if (stack->cap > 64 && stack->top <= stack->cap / 4) {
    stack->cap /= 2;
    stack->data = realloc(stack->data, stack->cap * sizeof(long));
  }
  return ret;
}

uint32_t eat(FILE *in, char target) {
  /* eat up to 2^32-1 characters to return the count */
  int c;
  uint32_t ret = 1; /* don't forget the byte initiated the process */
  while ((c = fgetc(in)) == target && ret < 0xffffffff) {
    ++ret;
  }
  if (c != EOF) {
    ungetc(c, in);
  }
  return ret;
}

void emit_32(FILE *out, uint32_t oprand) {
  fputc((uint8_t)(oprand >> 24), out);
  fputc((uint8_t)((oprand >> 16) & 0xff), out);
  fputc((uint8_t)((oprand >> 8) & 0xff), out);
  fputc((uint8_t)((oprand)&0xff), out);
}

void transform(FILE *in, FILE *out) {
  int c;
  stack_t st = stack_new();
  do {
    switch (c = fgetc(in)) {
      case '-': {
        fputc(I_DEC, out);
        uint32_t oprand = eat(in, '-');
        assert(oprand < 256);
        fputc((uint8_t)oprand, out);
        break;
      }
      case '+': {
        fputc(I_INC, out);
        uint32_t oprand = eat(in, '+');
        assert(oprand < 256);
        fputc((uint8_t)oprand, out);
        break;
      }
      case '<': {
        fputc(I_SHL, out);
        emit_32(out, eat(in, '<'));
        break;
      }
      case '>': {
        fputc(I_SHR, out);
        emit_32(out, eat(in, '>'));
        break;
      }
      case '[': {
        fputc(I_JZ, out);
        long pos = ftell(out);
        push(&st, pos);
        emit_32(out, 0); /* we will come back later */
        break;
      }
      case ']': {
        fputc(I_JNZ, out);
        long here = ftell(out);
        long there = pop(&st);
        long delta = here - there;
        fseek(out, there, SEEK_SET);
        emit_32(out, delta);
        fseek(out, here, SEEK_SET);
        emit_32(out, -delta);
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
      default:
        /* skip */
        break;
    }
  } while (c != EOF);
  fputc(I_EXIT, out);
  fflush(out);
}

#if 0
int main(int argc, char **argv) {
  if (argc < 2) {
    printf("fbcc [-otarget] source\n");
    exit(1);
  }
  char *tname = "a.bfc";
  int ch;
  while ((ch = getopt(argc, argv, "o::")) != -1) {
    switch (ch) {
      case 'o':
        tname = optarg;
        break;
      default:
        printf("fbcc [-otarget] source\n");
        exit(1);
    }
  }

  FILE *source = fopen(argv[optind], "r");
  if (!source) {
    perror("fopen");
    exit(1);
  }
  FILE *target = fopen(tname, "w+");
  if (!target) {
    perror("fopen");
    exit(1);
  }
  transform(source, target);
  fclose(source);
  fclose(target);
  return 0;
}
#endif
