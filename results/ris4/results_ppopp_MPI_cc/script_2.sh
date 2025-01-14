#!/bin/bash
#PBS -l select=2:ncpus=1:mem=25gb -l place=scatter:excl
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_ppopp_MPI_cc/output_2.txt
#PBS -e results_ppopp_MPI_cc/error_2.txt
module load mpich-3.2
mpirun.actual -n 2 ./ppopp_MPI_cc.out /shares/HPC4DataScience/big-graph-0.txt
