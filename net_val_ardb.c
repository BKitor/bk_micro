#include "bk_micro_util.h"

#define MAX_NODES 64
#define MAX_MSG_SIZE (1<<20)
#define NUM_ITTERS 128
#define START_MSG_SIZE (1<<10)
#define NUM_MAPPINGS 10

// ARDB - AllReduce/RecursiveDouble

/*
1. Allocate ranks block-bunch so they can be grouped into nodes (i.e., ranks 0-31 are node 0, 32-63 are node 1, etc.)
2. Measure the latency of node-wide all-reduce for a series of message sizes.
3. Reorder the nodes (i.e. ranks 0-31 become node 4, 32-63 become node 9, etc,)
4. Repeat 2. and 3. several times
*/

int main(int argc, char *argv[]){
    int rank, w_size, ppn, v_rank, com_rounds, i, partner, partner_n, node_r, warm_up;
    char *src_buff, *dst_buff;
    unsigned long align_size = sysconf(_SC_PAGESIZE), msg_size, max_mgs_size=MAX_MSG_SIZE;
    double itter_time;
    bk_opts opts;

    opts.n_nodes = 0;
    process_opts(argc, argv, &opts);

    srand(time(NULL));
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &w_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(opts.n_nodes == 0){
        W_P_MSG("node count is 0, aborting\n");
        goto abort;
    }

    if(opts.n_nodes > MAX_NODES){
        W_P_MSG("you've allocated to many nodes, aborting\n");
        goto abort;
    }

    // check if power of 2
    if(opts.n_nodes & (opts.n_nodes -1) != 0){
        W_P_MSG("n_nodes must be a power of 2\n");
        goto abort;
    }

    if(w_size%opts.n_nodes){
        W_P_MSG("Each node needs the same ammount of procs, aborting\n");
        goto abort;
    }

    if(posix_memalign((void**) &src_buff, align_size, max_mgs_size) ||
       posix_memalign((void**) &dst_buff, align_size, max_mgs_size)){
        W_P_MSG("memalloc failed, exiting/n");
        goto abort;
    }

    // log_2 int calculating, might want to make a util inline fn 
    com_rounds = 0;
    for(i = opts.n_nodes; i>1; i>>=1)com_rounds++;
    ppn = w_size/opts.n_nodes;
    node_r = rank/ppn;

    W_P_MSG("ardb called with %d rank and %d nodes and %d ppn\n", w_size, opts.n_nodes, ppn);
    fflush(stdout);

    int *mapping = calloc(opts.n_nodes, sizeof(int));
    int *partners = malloc(com_rounds*sizeof(int));
    int *tmp_arr = malloc(com_rounds*sizeof(int));

    for(int m = 0; m<NUM_MAPPINGS; m++){
        rand_arr(mapping, opts.n_nodes);
        MPI_Bcast(mapping, opts.n_nodes, MPI_INT, 0, MPI_COMM_WORLD);
        warm_up = 1;
        W_P_MSG("\nmappnig %d [ ", m);
        for(int j = 0; j<opts.n_nodes; j++)
            W_P_MSG("%d ", mapping[j]);
        W_P_MSG("]\n");

        // for(i = 0; i<com_rounds; i++){
        //     partner_n = node_r ^ (1<<i);
        //     partners[i] = ppn*partner_n + (rank%ppn);
        // }

        for(i = 0; i<com_rounds; i++){
            partner_n = mapping[node_r] ^ (1<<i);
            int virtual_partner = -1;
            for(int j = 0; j<opts.n_nodes; j++)
                if(mapping[j]==partner_n) virtual_partner=j;
            partners[i] = ppn*virtual_partner + (rank%ppn);
        }

        W_P_MSG("partenrs %d: [ ", rank);
        for(int j = 0; j<com_rounds; j++)
            W_P_MSG("%d ", partners[j]);
        W_P_MSG("]\n");
        fflush(stdout);

        for(int msg_size = START_MSG_SIZE>>1; msg_size < MAX_MSG_SIZE; msg_size<<=1){
            MPI_Barrier(MPI_COMM_WORLD);
            itter_time = -MPI_Wtime();
            for(int itter = 0; itter<NUM_ITTERS; itter++){
                for(i = 0; i<com_rounds; i++){
                    if(rank>partners[i]){
                        MPI_Send(src_buff, msg_size, MPI_BYTE, partners[i], 0, MPI_COMM_WORLD);
                        MPI_Recv(dst_buff, msg_size, MPI_BYTE, partners[i], 0,
                                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }else{
                        MPI_Recv(dst_buff, msg_size, MPI_BYTE, partners[i], 0,
                                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        MPI_Send(src_buff, msg_size, MPI_BYTE, partners[i], 0, MPI_COMM_WORLD);
                    }
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            itter_time += MPI_Wtime();
            if(warm_up)warm_up = 0; 
            else W_P_MSG("%-20d %.2f\n", msg_size, itter_time*1e6/NUM_ITTERS);
        }
    }

    MPI_Finalize();

    free(mapping);
    free(partners);
    free(dst_buff);
    free(src_buff);

    return 0;

    abort:
    free(dst_buff);
    free(src_buff);
    MPI_Finalize();
    return 1;
}
