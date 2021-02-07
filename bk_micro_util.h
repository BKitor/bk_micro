#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

typedef struct bk_opts {
    int n_nodes;
} bk_opts;

int process_opts(int argc, char** argv, bk_opts *opts);
void p_msg(const char *format, ...);
