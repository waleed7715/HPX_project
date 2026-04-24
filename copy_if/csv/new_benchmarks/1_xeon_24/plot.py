import pandas as pd
import matplotlib.pyplot as plt

seq       = pd.read_csv("seq.csv")
flag      = pd.read_csv("using_flag.csv").dropna(subset=["size"])

for df in [flag]:
    df["size"]    = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
seq["size"] = seq["size"].astype(int)

sizes       = [100_000, 10_000_000, 1_000_000_000]
size_labels = ["100,000", "10,000,000", "1,000,000,000"]
threads     = [1, 2, 4, 8, 12, 16, 24]

for size, label in zip(sizes, size_labels):
    fig, ax = plt.subplots(figsize=(10, 6))

    seq_min    = seq.loc[seq["size"] == size, "min"].values[0]
    flag_rows  = flag[flag["size"] == size].set_index("threads")

    flag_speedup = [seq_min / flag_rows.loc[t, "min"] for t in threads]

    ax.plot(threads, flag_speedup, marker="o", color="#1f77b4", linewidth=2.5, markersize=8, label="--hpx:threads")

    # ✅ Only annotate first, middle, last
    indices = [len(threads)-1]

    for i in indices:
        t  = threads[i]
        fs = flag_speedup[i]

        # Flag (above-right)
        ax.annotate(f"{fs:.2f}×",
                    (t, fs),
                    textcoords="offset points",
                    xytext=(-5, -5),
                    ha="left",
                    fontsize=12,
                    color="#1f77b4",
                    bbox=dict(boxstyle="round,pad=0.2", fc="white", alpha=0.7))

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
    plt.savefig(f"speedup_{size}.png", dpi=300)
