#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpi.h"

#define START_MSG_SIZE (2<<10)
#define MAX_MSG_SIZE (2<<22)
#define NUM_ITTERS 128
#define NUM_REQ 64

int main(int argc, char *argv[]){
    int rank, w_size, mid_rank, i, j;
    int comm_pair, *src_buff=NULL, *dst_buff=NULL;
    unsigned long align_size = sysconf(_SC_PAGESIZE), msg_size, max_mgs_size=MAX_MSG_SIZE;
    double t_start=0.0, t_end=0.0, bw_tmp;
    MPI_Status reqstats[100];
    MPI_Request requests[100];


    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &w_size);
    if(rank == 0){
        printf("This should be run with BLOCK-BUNCH mapping "
        "(--map-by core) on OpenMPI \n" 
        "This also expects 2 nodes, with the same ammount of cores per node\n");
        printf("\nNET_VAL_BW with world size %d\n", w_size);
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
            if(rank<mid_rank){
                for(j = 0; j<NUM_REQ; j++){
                    MPI_Isend(src_buff, msg_size, MPI_BYTE, comm_pair, 0, MPI_COMM_WORLD, &requests[j]);
                }
                MPI_Waitall(NUM_REQ, requests, reqstats);
                MPI_Recv(dst_buff, 4, MPI_CHAR, comm_pair, 1, MPI_COMM_WORLD, &reqstats[0]);

            }else{
                for(j = 0; j<NUM_REQ; j++){
                    MPI_Irecv(dst_buff, msg_size, MPI_BYTE, comm_pair, 0, MPI_COMM_WORLD, &requests[j]);
                }
                MPI_Waitall(NUM_REQ, requests, reqstats);
                MPI_Send(src_buff, 4, MPI_CHAR, comm_pair, 1, MPI_COMM_WORLD);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        t_end = MPI_Wtime();
        if(rank == 0){
            bw_tmp = (msg_size/1e6 * NUM_ITTERS * NUM_REQ)/(t_end - t_start);
            printf("%-20d%.2f\n", msg_size, bw_tmp);
        }
    }

    free(dst_buff);
    free(src_buff);

    MPI_Finalize();
    return 0;
}
