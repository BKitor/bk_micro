#include "bk_micro_util.h"

void p_msg(const char *format, ...){
    va_list args;
    va_start(args, format);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
        vprintf(format, args);

    va_end(args);
}

int process_opts(int argc, char** argv, bk_opts *opts){
    int c;
    while((c = getopt(argc, argv, "n:"))!= -1)
        switch(c){
            case 'n':
            opts->n_nodes = atoi(optarg);
            break;
        }
}