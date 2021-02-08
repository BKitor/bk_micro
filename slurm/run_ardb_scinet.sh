#!/usr/bin/env bash
#SBATCH --nodes=32
#SBATCH --ntasks-per-node=32
#SBATCH --time=4:00:00
#SBATCH --output=net_val_ardb-%j.txt
#SBATCH --error=net_val_ardb-error-%j.txt
#SBATCH --partition=debug
#SBATCH --account=<add your own>

cd $HOME/Niagara/bk_micro
source init_env.conf

if [ -z $SLURM_NTASKS ]; then
	echo "On a login node"
	SLURM_NTASKS=12
	SLURM_NNODES=4
else
	echo ""
fi

mpirun -n $SLURM_NTASKS --rank-by core --bind-to core:REPORT ./net_val_ardb.out -n $SLURM_NNODES

