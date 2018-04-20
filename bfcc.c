#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
  I_CLR
};

typedef struct stack {
  size_t cap;
  size_t top;
  long *data;
} stack_t;

stack_t stack_new() {
#define DEFAULT_SIZE 64
  long *data = malloc(sizeof(long) * DEFAULT_SIZE);
  stack_t ret = {
    .cap = DEFAULT_SIZE,
    .top = 0,
    .data = data
  };
  return ret;
}

void push(stack_t *stack, long pos) {
  if (stack->top >= stack->cap) {
    stack->cap *= 2;
    stack->data = realloc(stack->data, stack->cap);
  } 
  stack->data[stack->top++] = pos;
}

long pop(stack_t *stack) {
  int ret = stack->data[--stack->top];
  if (stack->top <= stack->cap / 4) {
    stack->cap /= 2;
    stack->data = realloc(stack->data, stack->cap);
  }
  return ret;
}

uint8_t eat(FILE *in, char target) {
  /* eat up to 255 characters to return the count */
  int c;
  uint8_t ret = 1; /* don't forget the byte initiated the process */
  while ((c = fgetc(in)) == target && ret <= 255) {
    ++ret;
  }
  if (c != EOF) {
    ungetc(c, in);
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
        fputc(eat(in, '-'), out);
	break;
      }
      case '+': {
        fputc(I_INC, out);
        fputc(eat(in, '+'), out);
	break;
      }
      case '<': {
        fputc(I_SHL, out);
        fputc(eat(in, '<'), out);
	break;
      }
      case '>': {
        fputc(I_SHR, out);
        fputc(eat(in, '>'), out);
	break;
      }
      case '[': {
        fputc(I_JZ, out);
	long pos = ftell(out);
	push(&st, pos);
	fputc(0, out); /* we will come back later */
	break;
      }
      case ']': {
        fputc(I_JNZ, out);
        long here = ftell(out);
        long there = pop(&st);
        long delta = here - there;
        if (delta > 0xff) {
          printf("you cannot jump away more than 255 bytes");
          exit(1);
        }
        fseek(out, there, SEEK_SET);
        fputc(delta, out);
        fseek(out, here, SEEK_SET);
        fputc((uint8_t) -delta, out);
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
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("fbcc [-otarget] source\n");
    exit(1);
  }
  char *tname = "a.bfc";
  int ch;
  while ((ch = getopt(argc, argv, "o::")) != -1) {
    switch(ch) {
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
