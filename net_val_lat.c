#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

#define MAX_MSG_SIZE (2<<22)
#define NUM_ITTERS 128
#define START_MSG_SIZE (2<<10)

int main(int argc, char *argv[]){
    int rank, w_size, mid_rank, i;
    int comm_pair, *src_buff=NULL, *dst_buff=NULL;
    unsigned long align_size = sysconf(_SC_PAGESIZE), msg_size, max_mgs_size=MAX_MSG_SIZE;
    double t_start=0.0, t_end=0.0, lat_tmp;
    MPI_Status req_stat;


    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &w_size);
    if(rank == 0){
        printf("This should be run with BLOCK-BUNCH mapping "
        "(--map-by core) on OpenMPI \n" 
        "This also expects 2 nodes, with the same ammount of cores per node\n");
        printf("\nNET_VAL_LAT with world size %d\n", w_size);
    }
    if(w_size%2!=0){
        printf("world size must be even or else I will hang");
        return 1;
    }


    mid_rank = w_size/2; 
    comm_pair = (rank+mid_rank)%w_size;

    if(posix_memalign((void**) &src_buff, align_size, max_mgs_size) ||
       posix_memalign((void**) &dst_buff, align_size, max_mgs_size)){
        printf("memalloc failed, exiting/n");
        return 1;
    }

    for(msg_size = START_MSG_SIZE; msg_size <= MAX_MSG_SIZE; msg_size*=2){
        memset(src_buff, 'a', msg_size);
        memset(dst_buff, 'b', msg_size);

        MPI_Barrier(MPI_COMM_WORLD);
        t_start = MPI_Wtime();
        
        for(i = 0; i<NUM_ITTERS; i++){
            if(rank < mid_rank){
                MPI_Send(src_buff, msg_size, MPI_BYTE, comm_pair, 0, MPI_COMM_WORLD);
                MPI_Recv(dst_buff, msg_size, MPI_BYTE, comm_pair, 0, MPI_COMM_WORLD, &req_stat);
            }
            else{
                MPI_Recv(dst_buff, msg_size, MPI_BYTE, comm_pair, 0, MPI_COMM_WORLD, &req_stat);
                MPI_Send(src_buff, msg_size, MPI_BYTE, comm_pair, 0, MPI_COMM_WORLD);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        t_end = MPI_Wtime();
        if(rank == 0){
            lat_tmp = (t_end - t_start) * 1e6/(2.0*NUM_ITTERS);
            printf("%-20d%.2f\n", msg_size, lat_tmp);
        }
    }

    free(dst_buff);
    free(src_buff);

    MPI_Finalize();
    return 0;
}
