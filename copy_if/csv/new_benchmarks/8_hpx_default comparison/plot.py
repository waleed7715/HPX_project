import pandas as pd
import matplotlib.pyplot as plt

seq = pd.read_csv("seq.csv")
seq = seq.set_index("size")["min"].to_dict()

def load(path):
    df = pd.read_csv(path)
    df["size"] = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
    return df.set_index(["size", "threads"])["min"]

flag_c1  = load("using_num_chunks_1.csv")
flag_c2  = load("using_num_chunks_2.csv")
flag_def = load("using_num_chunks_default.csv")
tbb_par  = load("tbb_data_par.csv")
openmp_par = load("openmp_data_par.csv")
taskflow_par = load("taskflow_data_par.csv")

threads = [1, 2, 4, 8, 12, 16, 24]

series = [
    ("chunks=1", flag_c1,  "#1f77b4", "-",  "o"),
    ("chunks=2", flag_c2,  "#ff7f0e", "--", "s"),
    ("default",  flag_def, "#2ca02c", ":",  "^"),
    ("TBB par",  tbb_par,  "#d62728", "-.", "D"),
    ("OpenMP par", openmp_par, "#9467bd", "-.", "D"),
    ("Taskflow par", taskflow_par, "#8c564b", "-.", "D"),
]

sizes       = [100_000, 10_000_000, 1_000_000_000]
size_labels = ["100,000", "10,000,000", "1,000,000,000"]

for size, label in zip(sizes, size_labels):

    fig, ax = plt.subplots(figsize=(10, 6))

    base = seq[size]

    for name, s, color, ls, marker in series:

        speedup = [base / s.loc[(size, t)] for t in threads]

        ax.plot(
            threads,
            speedup,
            marker=marker,
            color=color,
            linestyle=ls,
            linewidth=2.5,
            markersize=8,
            label=name,
        )

        # ✅ Annotate ONLY last point, below it
        t  = threads[-1]
        sp = speedup[-1]

        ax.annotate(
            f"{sp:.2f}×",
            (t, sp),
            textcoords="offset points",
            xytext=(0, -18),   # directly below
            ha="center",
            fontsize=12,
            color=color,
            bbox=dict(boxstyle="round,pad=0.25", fc="white", alpha=0.8),
        )

    ax.axhline(
        y=1.0,
        color="black",
        linewidth=1,
        linestyle=":",
        label="Sequential (1×)"
    )

    ax.set_title(f"--hpx:threads flag — Speedup vs Threads\nInput Size = {label}",
                 fontsize=13, fontweight="bold")
    ax.set_xlabel("Number of Threads", fontsize=11)
    ax.set_ylabel("Speedup vs std::copy_if (×)", fontsize=11)
    ax.set_xticks(threads)
    ax.grid(True, alpha=0.3)
    ax.set_ylim(bottom=0)

    ax.legend(fontsize=10)

    fig.tight_layout()

    fig.savefig(
        f"speedup_flag_{size}.png",
        dpi=150,
        bbox_inches="tight"
    )
