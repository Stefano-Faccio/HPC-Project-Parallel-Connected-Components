#!/bin/bash
#PBS -l select=4:ncpus=1:mem=25gb -l place=scatter
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_ppopp_MPI_cc.out/output_4.txt
#PBS -e results_ppopp_MPI_cc.out/error_4.txt
module load mpich-3.2
mpirun.actual -n 4 ./ppopp_MPI_cc.out /shares/HPC4DataScience/big-graph-0.txt
