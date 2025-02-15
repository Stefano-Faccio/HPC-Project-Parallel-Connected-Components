#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

SERIAL = 65
n_proc = np.array([1, 2, 4, 8, 16, 32, 64])

MPI_ppOpp = [87, 71, 52, 48, 37, 34, 35]
MPI_my = [177, 108, 95, 52, 44, 40, 42]
OPENMP_det = [168, 175, 117, 109, 84, 66, 52]
OPENMP_rand = [1061, 747, 481, 413, 355, 281, 239]

# Calc Speedup
speedup_ppOpp = [SERIAL / MPI_ppOpp[i] for i in range(len(MPI_ppOpp))]
speedup_my = [SERIAL / MPI_my[i] for i in range(len(MPI_my))]   
speedup_det = [SERIAL / OPENMP_det[i] for i in range(len(OPENMP_det))]
speedup_rand = [SERIAL / OPENMP_rand[i] for i in range(len(OPENMP_rand))]


# Plot Speedup
plt.figure(figsize=(6, 6))
plt.plot(n_proc[0:len(speedup_ppOpp)], speedup_ppOpp, marker='o', label='PPoPP 2018 MPI')
plt.plot(n_proc[0:len(speedup_my)], speedup_my, marker='o', label='Deterministic MPI')
plt.plot(n_proc[0:len(speedup_det)], speedup_det, marker='o', label='Deterministic OpenMP')
plt.plot(n_proc[0:len(speedup_rand)], speedup_rand, marker='o', label='Randomized OpenMP')
plt.xticks(n_proc[1:len(n_proc)])
plt.xlabel('Number of processors')
plt.ylabel('Speedup')
#plt.title('Speedup')
plt.grid()
plt.legend()
plt.show()

# Calc Efficiency
efficiency_my = [speedup_my[i] / n_proc[i] for i in range(len(speedup_my))]
efficiency_ppOpp = [speedup_ppOpp[i] / n_proc[i] for i in range(len(speedup_ppOpp))]
efficiency_det = [speedup_det[i] / n_proc[i] for i in range(len(speedup_det))]
efficiency_rand = [speedup_rand[i] / n_proc[i] for i in range(len(speedup_rand))]

# Plot Efficiency
plt.figure(figsize=(6, 6))

plt.plot(n_proc[0:len(speedup_ppOpp)], efficiency_ppOpp, marker='o', label='PPoPP 2018 MPI')
plt.plot(n_proc[0:len(speedup_my)], efficiency_my, marker='o', label='Deterministic MPI')
plt.plot(n_proc[0:len(speedup_det)], efficiency_det, marker='o', label='Deterministic OpenMP')
plt.plot(n_proc[0:len(speedup_rand)], efficiency_rand, marker='o', label='Randomized OpenMP')
plt.xlabel('Number of processors')
plt.ylabel('Efficiency')
#plt.title('Efficiency')
plt.xticks(n_proc[1:len(n_proc)])
plt.grid()
plt.legend()
plt.show()





