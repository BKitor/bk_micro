#!/usr/bin/bash

if ! hash mpirun 2> /dev/null; then
    echo "mpirun not found"
    exit
fi

if ! hash mpicc 2> /dev/null; then
    echo "mpicc not found"
    exit
fi

if ! make net_val_bw_mul 2> /dev/null; then
    echo "failed to build"
    exit
fi
# mpirun -n 12 ./net_val_bw_mul.out -n 2
# mpirun -n 12 ./net_val_bw_mul.out -n 3
mpirun -n 12 ./net_val_bw_mul.out -n 4