#!/bin/bash
#PBS -l select=1:ncpus=2:mem=100gb
#PBS -l walltime=01:00:00
#PBS -q short_cpuQ
#PBS -o results_randomized_OPENMP_cc.out/output_2.txt
#PBS -e results_randomized_OPENMP_cc.out/error_2.txt
module load mpich-3.2
./randomized_OPENMP_cc.out /shares/HPC4DataScience/big-graph-0.txt
