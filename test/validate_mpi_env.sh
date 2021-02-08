#!/usr/bin/bash

if ! hash mpirun 2> /dev/null; then
    echo "mpirun not found"
    exit 1
fi

if ! hash mpicc 2> /dev/null; then
    echo "mpicc not found"
    exit 1
fi