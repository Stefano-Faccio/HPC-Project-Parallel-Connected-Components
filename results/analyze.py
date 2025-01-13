#!/usr/bin/env python3

import matplotlib.pyplot as plt

n_proc = [1, 2, 4, 8, 16, 32, 64]
MPI_my = [355, 288, 200, 244, 220, 187, 212]
MPI_ppOpp = [139, 103, 60, 48, 42, 41, 49]
OPENMP_rand = [1061, 747, 481, 413, 363, 384, 395]
OPENMP_det = [308, 177, 125 , 124, 97, 94, 95]
SERIAL = 51.08

# Calcolo Speedup come tempo sequenziale / tempo parallelo
speedup_my = [SERIAL / MPI_my[i] for i in range(len(MPI_my))]   
speedup_ppOpp = [SERIAL / MPI_ppOpp[i] for i in range(len(MPI_ppOpp))]
speedup_rand = [SERIAL / OPENMP_rand[i] for i in range(len(OPENMP_rand))]
speedup_det = [SERIAL / OPENMP_det[i] for i in range(len(OPENMP_det))]

# Calcolo Efficienza come Speedup / numero di processori
efficiency_my = [speedup_my[i] / n_proc[i] for i in range(len(n_proc))]
efficiency_ppOpp = [speedup_ppOpp[i] / n_proc[i] for i in range(len(n_proc))]
efficiency_rand = [speedup_rand[i] / n_proc[i] for i in range(len(n_proc))]
efficiency_det = [speedup_det[i] / n_proc[i] for i in range(len(n_proc))]

# Plot Speedup
plt.figure(figsize=(10, 6))
plt.plot(n_proc, speedup_my, marker='o', label='MPI_my')
plt.plot(n_proc, speedup_ppOpp, marker='o', label='MPI_ppOpp')
plt.plot(n_proc, speedup_rand, marker='o', label='OPENMP_rand')
plt.plot(n_proc, speedup_det, marker='o', label='OPENMP_det')
plt.xlabel('Numero di processori')
plt.ylabel('Speedup')
plt.title('Speedup')
plt.grid()
plt.legend()
plt.show()

# Plot Efficienza
plt.figure(figsize=(10, 6))
plt.plot(n_proc, efficiency_my, marker='o', label='MPI_my')
plt.plot(n_proc, efficiency_ppOpp, marker='o', label='MPI_ppOpp')
plt.plot(n_proc, efficiency_rand, marker='o', label='OPENMP_rand')
plt.plot(n_proc, efficiency_det, marker='o', label='OPENMP_det')
plt.xlabel('Numero di processori')
plt.ylabel('Efficienza')
plt.title('Efficienza')
plt.grid()
plt.legend()
plt.show()





