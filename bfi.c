/*
 * Brainf*ck interpreter.
 *
 * Copyright (c) 2019 Zeling Feng
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bf.h"
#include "jit.h"

static void die(const char *msg)
{
    perror(msg);
    exit(1);
}

void usage(const char *name)
{
    printf("%s [-j] [-n npage] [file]\n", name);
    exit(2);
}

int main(int argc, char **argv)
{
    FILE *src;
    bf_t bf;
    int opt, jit = 0, npage = 2;

    while ((opt = getopt(argc, argv, "jp:")) != -1) {
        switch (opt) {
        case 'j':
            jit = 1;
            break;
        case 'p':
            npage = atoi(optarg);
            if (npage <= 0) {
                puts("npage must be positive");
                usage(argv[0]);
            }
            break;
        default:
            printf("Unrecognized option: %c\n", opt);
            usage(argv[0]);
        }
    }
    if (optind >= argc) {
        src = stdin;
    } else {
        src = fopen(argv[optind], "r");
        if (!src) {
            die("fopen");
        }
    }
    if (jit) {
        bf_init_jit(&bf);
    } else {
        bf_init(&bf);
    }
    if (bf_load_file(&bf, src) < 0) {
        die("bf_load_file");
    }
    if (bf_run(&bf, npage) < 0) {
        die("bf_run");
    }
    bf_free(&bf);
    return 0;
}
