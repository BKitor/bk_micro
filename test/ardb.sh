#!/usr/bin/bash

if ! ./test/validate_mpi_env.sh; then 
    exit 
fi

if ! make net_val_ardb; then
    echo "failed to build"
    exit
fi

# mpirun -n 12 ./net_val_ardb.out -n 4
# echo "****"
mpirun -n 16 ./net_val_ardb.out -n 8
