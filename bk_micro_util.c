#include "bk_micro_util.h"

void p_msg(MPI_Comm comm, const char *format, ...){
    va_list args;
    va_start(args, format);

    int rank;
    MPI_Comm_rank(comm, &rank);

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

// Pointer to an array of ints, and the size to be retured
// fill arr with random numbers from 0 to n-1
void rand_arr(int * arr, int n){
    int inner_size = n, r, swp_tmp;
    int *inner_arr = malloc(sizeof(int)*n);

    for(int i = 0; i<inner_size; i++)
        inner_arr[i]=i;

    for(int i = 0; i<n; i++){
        r = rand() % inner_size;
        arr[i] = inner_arr[r];

        swp_tmp = inner_arr[inner_size - 1];
        inner_arr[inner_size - 1] = inner_arr[r];
        inner_arr[r] = swp_tmp;
        inner_size--;
    }

    free(inner_arr);
}