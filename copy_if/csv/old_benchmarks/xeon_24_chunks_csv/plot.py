import pandas as pd
import matplotlib.pyplot as plt

seq = pd.read_csv("seq.csv")
seq = seq.set_index("size")["min"].to_dict()

def load(path):
    df = pd.read_csv(path)
    df["size"] = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
    return df.set_index(["size", "threads"])["min"]

flag_c1  = load("using_flag_chunk_1.csv")
flag_c2  = load("using_flag_chunk_2.csv")
flag_def = load("using_flag_default.csv")

nc_c1  = load("using_num_cores_chunk_1.csv")
nc_c2  = load("using_num_cores_chunk_2.csv")
nc_def = load("using_num_cores.csv")

threads = [1, 2, 4, 8, 16, 24]

groups = [
    ("--hpx:threads flag", [
        ("chunks=1", flag_c1,  "#1f77b4", "-",  "o",  10),
        ("chunks=2", flag_c2,  "#ff7f0e", "--", "s", -16),
        ("default",  flag_def, "#2ca02c", ":",  "^",  10),
    ]),
    ("num_cores", [
        ("chunks=1", nc_c1,  "#1f77b4", "-",  "o",  10),
        ("chunks=2", nc_c2,  "#ff7f0e", "--", "s", -16),
        ("default",  nc_def, "#2ca02c", ":",  "^",  10),
    ]),
]

sizes       = [100_000, 10_000_000, 1_000_000_000]
size_labels = ["100,000", "10,000,000", "1,000,000,000"]

for size, label in zip(sizes, size_labels):

    fig, axes = plt.subplots(1, 2, figsize=(18, 6), sharey=True)

    base = seq[size]

    for ax, (group_title, series) in zip(axes, groups):

        for name, s, color, ls, marker, offset in series:

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

            for t, sp in zip(threads, speedup):
                ax.annotate(
                    f"{sp:.2f}×",
                    (t, sp),
                    textcoords="offset points",
                    xytext=(0, offset),
                    ha="center",
                    fontsize=7,
                    color=color,
                )

        ax.axhline(
            y=1.0,
            color="black",
            linewidth=1,
            linestyle=":",
            label="Sequential (1×)"
        )

        ax.set_title(group_title, fontsize=12, fontweight="bold")
        ax.set_xlabel("Number of Threads", fontsize=11)
        ax.set_ylabel("Speedup vs std::copy_if (×)", fontsize=11)
        ax.set_xticks(threads)
        ax.grid(True, alpha=0.3)
        ax.set_ylim(bottom=0)

        ax.legend(fontsize=9)

    fig.suptitle(f"Speedup vs Threads (Input Size = {label})", fontsize=14)

    fig.tight_layout()

    fig.savefig(
        f"speedup_{size}.png",
        dpi=150,
        bbox_inches="tight"
    )

    plt.show()
