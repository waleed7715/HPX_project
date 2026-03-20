import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ── Load data ────────────────────────────────────────────────────────────────
seq            = pd.read_csv("seq.csv")
bulk_flag      = pd.read_csv("using_bulk_async_flag.csv")
bulk_numcores  = pd.read_csv("using_bulk_async_num_cores.csv")

for df in [bulk_flag, bulk_numcores]:
    df["size"]    = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)

seq["size"] = seq["size"].astype(int)

SIZES = sorted(bulk_flag["size"].unique())

SIZE_LABELS = {
    100_000: "100K",
    10_000_000: "10M",
    1_000_000_000: "1B"
}

COLORS = {
    "flag": "#2196F3",
    "cores": "#FF5722"
}

MARKERS = {
    "flag": "o",
    "cores": "s"
}

# ── Generate one plot per input size ─────────────────────────────────────────
for size in SIZES:

    fig, ax = plt.subplots(figsize=(7,5))

    seq_val = seq.loc[seq["size"] == size, "min"].values[0]

    sub_flag  = bulk_flag[bulk_flag["size"] == size].sort_values("threads")
    sub_cores = bulk_numcores[bulk_numcores["size"] == size].sort_values("threads")

    threads = sub_flag["threads"].values

    speedup_flag  = seq_val / sub_flag["min"].values
    speedup_cores = seq_val / sub_cores["min"].values

    ax.plot(
        threads,
        speedup_flag,
        marker=MARKERS["flag"],
        color=COLORS["flag"],
        linewidth=2,
        markersize=7,
        label="bulk_async_flag"
    )

    ax.plot(
        threads,
        speedup_cores,
        marker=MARKERS["cores"],
        color=COLORS["cores"],
        linewidth=2,
        markersize=7,
        label="bulk_async_num_cores"
    )

    ax.axhline(
        1.0,
        color="#607D8B",
        linestyle="--",
        linewidth=1.5,
        label="Baseline (seq = 1×)"
    )

    ax.set_xticks(threads)
    ax.set_xlabel("Threads", fontsize=11)
    ax.set_ylabel("Speedup (×)", fontsize=11)

    ax.set_title(
        f"Speedup over Sequential — Input size: {SIZE_LABELS[size]}",
        fontsize=13,
        fontweight="bold"
    )

    ax.grid(True, linestyle="--", alpha=0.4)

    ax.legend(fontsize=10)

    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.2f×"))

    filename = f"speedup_{SIZE_LABELS[size]}.png"

    plt.tight_layout()
    plt.savefig(filename, dpi=150, bbox_inches="tight")

    print(f"Saved: {filename}")

print("Done.")
