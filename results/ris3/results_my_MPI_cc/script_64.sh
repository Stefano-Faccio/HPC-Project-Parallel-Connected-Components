#!/bin/bash
#PBS -l select=64:ncpus=1:mem=40gb -l place=scatter
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_my_MPI_cc.out/output_64.txt
#PBS -e results_my_MPI_cc.out/error_64.txt
module load mpich-3.2
mpirun.actual -n 64 ./my_MPI_cc.out /shares/HPC4DataScience/big-graph-0.txt
