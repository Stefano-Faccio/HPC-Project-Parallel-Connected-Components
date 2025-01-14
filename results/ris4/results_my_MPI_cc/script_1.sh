#!/bin/bash
#PBS -l select=1:ncpus=1:mem=40gb -l place=scatter:excl
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_my_MPI_cc/output_1.txt
#PBS -e results_my_MPI_cc/error_1.txt
module load mpich-3.2
mpirun.actual -n 1 ./my_MPI_cc.out /shares/HPC4DataScience/big-graph-0.txt
