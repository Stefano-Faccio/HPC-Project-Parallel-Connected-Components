#!/bin/bash
#PBS -l select=32:ncpus=1:mem=35gb -l place=scatter
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_my_MPI_cc.out/output_32.txt
#PBS -e results_my_MPI_cc.out/error_32.txt
module load mpich-3.2
mpirun.actual -n 32 ./my_MPI_cc.out /shares/HPC4DataScience/big-graph-0.txt
