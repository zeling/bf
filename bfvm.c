#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

extern uint8_t *pc;
extern char *sp;

void interp();

void load_and_interp(const char *fname) {
  int fd = open(fname, O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }
  struct stat st;
  fstat(fd, &st);
  void *insts = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (insts == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
  size_t page_size = getpagesize();
  void *data = mmap(0, page_size * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
  mprotect(data, page_size, PROT_NONE);
  mprotect(data + 3 * page_size, page_size, PROT_NONE);

  pc = (uint8_t *) insts;
  sp = (char *) data + 2 * page_size;
  interp();
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    puts("bfvm bfcfile");
    exit(1);
  }
  load_and_interp(argv[1]);
  return 0;
}
