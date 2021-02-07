#include "bk_micro_util.h"

#define MAX_MSG_SIZE (2<<22)
#define NUM_ITTERS 128
#define START_MSG_SIZE (2<<10)

int main(int argc, char *argv[]){
    int rank, w_size, cutoff_rank, i, n, comm_key, k;
    int comm_pair, *src_buff=NULL, *dst_buff=NULL;
    unsigned long align_size = sysconf(_SC_PAGESIZE), msg_size, max_mgs_size=MAX_MSG_SIZE;
    double t_start=0.0, t_end=0.0, lat_tmp;
    bk_opts opts;
    MPI_Status req_stat;
    MPI_Comm comms[64];

    opts.n_nodes = 0;
    process_opts(argc, argv, &opts);

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &w_size);

    p_msg("This should be run with BLOCK-BUNCH mapping (--map-by core on OpenMPI) \n" 
    "This also expects 2 nodes, with the same ammount of cores per node\n");
    p_msg("\nNET_VAL_LAT with world size %d and %d nodes\n", w_size, opts.n_nodes);

    if(opts.n_nodes == 0){
        p_msg("node count is 0, aborting\n");
        goto abort;
    }

    if(opts.n_nodes > 64){
        p_msg("you've allocated to many nodes, aborting\n");
        goto abort;
    }

    if(w_size%opts.n_nodes){
        p_msg("Each node needs the same ammount of procs, aborting\n");
        goto abort;
    }

    if(posix_memalign((void**) &src_buff, align_size, max_mgs_size) ||
       posix_memalign((void**) &dst_buff, align_size, max_mgs_size)){
        p_msg("memalloc failed, exiting/n");
        goto abort;
    }

    cutoff_rank = w_size/opts.n_nodes;
    comm_key = rank/cutoff_rank;

    for(k = 1; k<opts.n_nodes; k++){
        p_msg("\nRunning against k:%d\n", k);
        if((rank < cutoff_rank) || ((rank/cutoff_rank) == k)){// in this cycle
            comm_pair = (rank<cutoff_rank) ? rank + (k*cutoff_rank) 
                        : rank % cutoff_rank;

            for(msg_size = START_MSG_SIZE; msg_size <= MAX_MSG_SIZE; msg_size*=2){
                memset(src_buff, 'a', msg_size);
                memset(dst_buff, 'b', msg_size);

                MPI_Barrier(MPI_COMM_WORLD);
                t_start = MPI_Wtime();
                    
                for(i = 0; i<NUM_ITTERS; i++){
                    if(rank < cutoff_rank){
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

        }else{//strait to barrier
            for(msg_size = START_MSG_SIZE; msg_size <=MAX_MSG_SIZE; msg_size*=2){
                MPI_Barrier(MPI_COMM_WORLD);
                MPI_Barrier(MPI_COMM_WORLD);
            }
        }
    }

    free(dst_buff);
    free(src_buff);

    MPI_Finalize();
    return 0;

    abort:
    free(dst_buff);
    free(src_buff);
    MPI_Finalize();
    return 1;
}
