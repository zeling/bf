#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>

int load_and_interp(const char *fname) {
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
}
