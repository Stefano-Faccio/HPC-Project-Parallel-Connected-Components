#!/bin/bash

# 2 nodes, 10 cores per node, 2GB memory per node
#PBS -l select=2:ncpus=10:mem=2gb

# 1 minute walltime
#PBS -l walltime=00:01:00

# execution queue
#PBS -q short_cpuQ

# specify the output file
#PBS -o output.txt

# specify the error file
#PBS -e error.txt

module load mpich-3.2
# 20 because 2 nodes * 10 cores per node
mpirun.actual -n 20 ./parallel_cc.out input/medium1.txt