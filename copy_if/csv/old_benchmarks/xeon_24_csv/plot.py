import pandas as pd
import matplotlib.pyplot as plt

seq       = pd.read_csv("seq.csv")
flag      = pd.read_csv("using_flag.csv").dropna(subset=["size"])
num_cores = pd.read_csv("using_num_cores.csv").dropna(subset=["size"])

for df in [flag, num_cores]:
    df["size"]    = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
seq["size"] = seq["size"].astype(int)

sizes       = [100_000, 10_000_000, 1_000_000_000]
size_labels = ["100,000", "10,000,000", "1,000,000,000"]
threads     = [1, 2, 4, 8, 16, 24]

for size, label in zip(sizes, size_labels):
    fig, ax = plt.subplots(figsize=(10, 6))

    seq_min    = seq.loc[seq["size"] == size, "min"].values[0]
    flag_rows  = flag[flag["size"] == size].set_index("threads")
    nc_rows    = num_cores[num_cores["size"] == size].set_index("threads")

    flag_speedup = [seq_min / flag_rows.loc[t, "min"] for t in threads]
    nc_speedup   = [seq_min / nc_rows.loc[t,   "min"] for t in threads]

    ax.plot(threads, flag_speedup, marker="o", color="#1f77b4", linewidth=2.5, markersize=8, label="--hpx:threads")
    ax.plot(threads, nc_speedup,   marker="s", color="#ff7f0e", linewidth=2.5, markersize=8, label="num_cores")

    # Annotate each point with its speedup value
    for t, fs, ns in zip(threads, flag_speedup, nc_speedup):
        ax.annotate(f"{fs:.2f}×", (t, fs), textcoords="offset points", xytext=(0, 10),
                    ha="center", fontsize=9, color="#1f77b4")
        ax.annotate(f"{ns:.2f}×", (t, ns), textcoords="offset points", xytext=(0, -16),
                    ha="center", fontsize=9, color="#ff7f0e")

    ax.axhline(y=1.0, color="black", linewidth=0.8, linestyle=":", label="Baseline (1×)")

    ax.set_title(f"HPX copy_if — Speedup vs Thread Count\nInput size: {label}",
                 fontsize=13, fontweight="bold", pad=15)
    ax.set_xlabel("Number of Threads", fontsize=12)
    ax.set_ylabel("Speedup (×)", fontsize=12)
    ax.set_xticks(threads)
    ax.legend(fontsize=11, loc="center right")
    ax.grid(True, alpha=0.3)
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    plt.show()
