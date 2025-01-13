#!/bin/bash
#PBS -l select=1:ncpus=16:mem=40gb
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_deterministic_OPENMP_cc.out/output_16.txt
#PBS -e results_deterministic_OPENMP_cc.out/error_16.txt
module load mpich-3.2
./deterministic_OPENMP_cc.out /shares/HPC4DataScience/big-graph-0.txt
