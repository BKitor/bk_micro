#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

#define W_P_MSG(...) p_msg(MPI_COMM_WORLD, __VA_ARGS__)

typedef struct bk_opts {
    int n_nodes;
} bk_opts;

int process_opts(int argc, char** argv, bk_opts *opts);
void p_msg(MPI_Comm comm, const char *format, ...);

void rand_arr(int * arr, int n);