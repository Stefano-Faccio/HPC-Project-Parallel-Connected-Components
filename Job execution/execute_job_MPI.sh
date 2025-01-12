#!/bin/bash

# Nome eseguibile
exec_name="parallel_cc.out"

# Nome grafo input
#input="/shares/HPC4DataScience/big-graph-0.txt"
input="/home/stefano.faccio/input/slide.txt"

# Quantità di memoria richiesta
ram=1

# Lista numero di processori
#cpus=(1 2 4 8 16 32 64 128)
cpus=(1)

# Walltime
walltime="00:01:00"

# Creo una cartella per gli script ed i risultati
mkdir -p "results_${exec_name}"

for cpu in ${cpus[@]}; do
    # Creo uno script per ogni numero di processori
    echo "#!/bin/bash" > "results_${exec_name}/script_${cpu}.sh"
    echo "#PBS -l select=${cpu}:ncpus=1:mem=${ram}gb -l place=scatter" >> "results_${exec_name}/script_${cpu}.sh"
    echo "#PBS -l walltime=${walltime}" >> "results_${exec_name}/script_${cpu}.sh"
    echo "#PBS -q short_cpuQ" >> "results_${exec_name}/script_${cpu}.sh"
    echo "#PBS -o results_${exec_name}/output_${cpu}.txt" >> "results_${exec_name}/script_${cpu}.sh"
    echo "#PBS -e results_${exec_name}/error_${cpu}.txt" >> "results_${exec_name}/script_${cpu}.sh"
    echo "module load mpich-3.2" >> "results_${exec_name}/script_${cpu}.sh"
    echo "mpirun.actual -n ${cpu} ./${exec_name} ${input}" >> "results_${exec_name}/script_${cpu}.sh"
    # Lancio lo script
    qsub "results_${exec_name}/script_${cpu}.sh"
done

# Aspetto 100 ms
sleep 0.1

qstat -u $USER