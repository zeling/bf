#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern uint8_t *pc;
extern char *sp;

void interp();

void load_and_interp(int fd)
{
    struct stat st;
    fstat(fd, &st);
    void *insts = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (insts == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    size_t page_size = getpagesize();
    void *data = mmap(0, page_size * 4, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    mprotect(data, page_size, PROT_NONE);
    mprotect(data + 3 * page_size, page_size, PROT_NONE);

    pc = (uint8_t *)insts;
    sp = (char *)data + 2 * page_size;
    interp();
}
