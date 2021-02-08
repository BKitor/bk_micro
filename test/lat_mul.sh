#!/usr/bin/bash

if ! ./test/validate_mpi_env.sh; then 
    exit 
fi

if ! make net_val_lat_mul 2> /dev/null; then
    echo "failed to build"
    exit
fi
# mpirun -n 12 ./net_val_lat_mul.out -n 2
# mpirun -n 12 ./net_val_lat_mul.out -n 3
mpirun -n 12 ./net_val_lat_mul.out -n 4
