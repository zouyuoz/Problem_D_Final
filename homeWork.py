import numpy as np
import matplotlib.pyplot as plt

def P_E_M_N(M, N):
    return ((N-M) * (N-1)) / (N * (N-2))

N_range = range(3, 100)
M_range = range(2, 99)

P_values = np.array([[P_E_M_N(M, N) if M < N else 0 for N in N_range] for M in M_range])

plt.figure(figsize=(12, 8))
plt.imshow(P_values, cmap='jet', aspect='auto', origin='lower')
plt.colorbar(label='P(E$_{M,N}$)')
plt.xlabel('N')
plt.ylabel('M')
plt.title('Heatmap of P(E$_{M,N}$)')
plt.xticks(range(0, len(N_range), 10), [str(N_range[i]) for i in range(0, len(N_range), 10)])
plt.yticks(range(0, len(M_range), 10), [str(M_range[i]) for i in range(0, len(M_range), 10)])
plt.show()