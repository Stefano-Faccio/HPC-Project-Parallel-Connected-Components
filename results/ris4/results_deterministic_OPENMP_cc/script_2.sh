#!/bin/bash
#PBS -l select=1:ncpus=2:mem=40gb -l place=excl
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_deterministic_OPENMP_cc/output_2.txt
#PBS -e results_deterministic_OPENMP_cc/error_2.txt
module load mpich-3.2
./deterministic_OPENMP_cc.out /shares/HPC4DataScience/big-graph-0.txt
