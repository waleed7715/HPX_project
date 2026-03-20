import matplotlib.pyplot as plt

threads = [1, 2, 4, 8, 16, 24, 32, 48]
seq_min = 441
tbb_min = [603, 254, 176, 171, 138, 121, 111, 107]
speedup = [seq_min / v for v in tbb_min]

fig, ax = plt.subplots(figsize=(8, 5))

ax.plot(threads, speedup, marker='o', color='#378ADD', linewidth=2, markersize=6, label='TBB (min)')

ax.set_xlabel('Threads')
ax.set_ylabel('Speedup (×)')
ax.set_title('TBB copy_if speedup — input size 100,000')
ax.set_xticks(threads)
ax.legend()
ax.grid(True, linestyle='--', alpha=0.4)

plt.tight_layout()
plt.savefig('tbb_speedup_100k.png', dpi=150)
plt.show()
