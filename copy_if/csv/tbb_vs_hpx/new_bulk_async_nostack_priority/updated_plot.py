import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

seq          = pd.read_csv("seq.csv")
default      = pd.read_csv("using_num_chunks_default.csv")
num_chunks_1 = pd.read_csv("using_num_chunks_1.csv")
num_chunks_2 = pd.read_csv("using_num_chunks_2.csv")
tbb_par      = pd.read_csv("tbb_data_par.csv")

for df in [default, num_chunks_1, num_chunks_2, seq, tbb_par]:
    df["size"] = df["size"].astype(int)

for df in [default, num_chunks_1, num_chunks_2, tbb_par]:
    df["threads"] = df["threads"].astype(int)

SIZES = sorted(default["size"].unique())

SIZE_LABELS = {
    100_000:       "100K",
    10_000_000:    "10M",
    1_000_000_000: "1B"
}

COLORS = {
    "default":  "#2196F3",
    "chunks_1": "#FF5722",
    "chunks_2": "#4CAF50",
    "tbb_par":  "#9C27B0"
}

MARKERS = {
    "default":  "o",
    "chunks_1": "s",
    "chunks_2": "^",
    "tbb_par":  "D"
}

for size in SIZES:
    fig, ax = plt.subplots(figsize=(7, 5))

    seq_val = seq.loc[seq["size"] == size, "min"].values[0]

    sub_default  = default[default["size"] == size].sort_values("threads")
    sub_chunks_1 = num_chunks_1[num_chunks_1["size"] == size].sort_values("threads")
    sub_chunks_2 = num_chunks_2[num_chunks_2["size"] == size].sort_values("threads")
    sub_tbb_par  = tbb_par[tbb_par["size"] == size].sort_values("threads")

    threads = sub_default["threads"].values

    speedup_default  = seq_val / sub_default["min"].values
    speedup_chunks_1 = seq_val / sub_chunks_1["min"].values
    speedup_chunks_2 = seq_val / sub_chunks_2["min"].values
    speedup_tbb_par  = seq_val / sub_tbb_par["min"].values

    ax.plot(threads, speedup_default,
            marker=MARKERS["default"], color=COLORS["default"],
            linewidth=2, markersize=7, label="bulk_async_nostack (default)")

    ax.plot(threads, speedup_chunks_1,
            marker=MARKERS["chunks_1"], color=COLORS["chunks_1"],
            linewidth=2, markersize=7, label="bulk_async_nostack (num_chunks=1)")

    ax.plot(threads, speedup_chunks_2,
            marker=MARKERS["chunks_2"], color=COLORS["chunks_2"],
            linewidth=2, markersize=7, label="bulk_async_nostack (num_chunks=2)")

    ax.plot(sub_tbb_par["threads"].values, speedup_tbb_par,
            marker=MARKERS["tbb_par"], color=COLORS["tbb_par"],
            linewidth=2, markersize=7, label="TBB PSTL (parallel scan)")

    ax.axhline(1.0, color="#607D8B", linestyle="--", linewidth=1.5,
               label="Baseline (seq = 1×)")

    ax.set_xticks(threads)
    ax.set_xlabel("Threads", fontsize=11)
    ax.set_ylabel("Speedup (×)", fontsize=11)
    ax.set_title(
        f"Speedup over Sequential — Input size: {SIZE_LABELS[size]}",
        fontsize=13, fontweight="bold"
    )
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend(fontsize=10)
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.2f×"))

    filename = f"speedup_{SIZE_LABELS[size]}.png"
    plt.tight_layout()
    plt.savefig(filename, dpi=150, bbox_inches="tight")
    print(f"Saved: {filename}")

print("Done.")
