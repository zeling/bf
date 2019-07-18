#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bf.h"

int main(int argc, char **argv)
{
    FILE *src;
    if (argc == 1) {
        src = stdin;
    } else {
        src = fopen(argv[1], "r");
        if (!src) {
            perror("fopen");
            exit(1);
        }
    }
    char tempname[] = "/tmp/bfcc-XXXXXX";
    int tfd = mkstemp(tempname);
    unlink(tempname);
    if (tfd == -1) {
        perror("mkstemp");
        exit(1);
    }
    FILE *dst = fdopen(tfd, "w+");
    if (!dst) {
        perror("fdopen");
        exit(1);
    }
    transform(src, dst);
    load_and_interp(tfd);
    return 0;
}
