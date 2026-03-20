import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ── Load data ────────────────────────────────────────────────────────────────
seq             = pd.read_csv("seq.csv")
async_flag      = pd.read_csv("using_async_flag.csv")
async_num_cores = pd.read_csv("using_async_num_cores.csv")
bulk_flag       = pd.read_csv("using_bulk_async_flag.csv")
bulk_num_cores  = pd.read_csv("using_bulk_async_num_cores.csv")

for df in [async_flag, async_num_cores, bulk_flag, bulk_num_cores]:
    df["size"]    = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
seq["size"] = seq["size"].astype(int)

SIZES       = sorted(async_flag["size"].unique())
SIZE_LABELS = {100_000: "100K", 10_000_000: "10M", 1_000_000_000: "1B"}
THREAD_COUNTS = sorted(async_flag["threads"].unique())

COLORS  = {"a": "#2196F3", "b": "#FF5722"}
MARKERS = {"a": "o",       "b": "s"}

def speedup_panel(df_a, df_b, label_a, label_b, size, seq_df, ax):
    seq_val = seq_df.loc[seq_df["size"] == size, "min"].values[0]
    sub_a   = df_a[df_a["size"] == size].sort_values("threads")
    sub_b   = df_b[df_b["size"] == size].sort_values("threads")

    threads  = sub_a["threads"].values
    speedup_a = seq_val / sub_a["min"].values
    speedup_b = seq_val / sub_b["min"].values

    ax.plot(threads, speedup_a, marker=MARKERS["a"], color=COLORS["a"],
            linewidth=2, markersize=7, label=label_a)
    ax.plot(threads, speedup_b, marker=MARKERS["b"], color=COLORS["b"],
            linewidth=2, markersize=7, label=label_b)
    ax.axhline(1.0, color="#607D8B", linestyle="--", linewidth=1.5,
               label="Baseline (seq = 1×)")

    ax.set_xticks(threads)
    ax.set_xticklabels(threads)
    ax.set_xlabel("Threads", fontsize=11)
    ax.set_ylabel("Speedup (×)", fontsize=11)
    ax.set_title(f"Input size: {SIZE_LABELS[size]}", fontsize=12, fontweight="bold")
    ax.legend(fontsize=9)
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.2f×"))

# ── Figure 1: flag variants ──────────────────────────────────────────────────
fig1, axes1 = plt.subplots(1, 3, figsize=(18, 5))
fig1.suptitle("Speedup over Sequential Using Flag (min time) — async vs bulk_async",
              fontsize=14, fontweight="bold", y=1.02)

for ax, size in zip(axes1, SIZES):
    speedup_panel(async_flag, bulk_flag,
                  "async_flag", "bulk_async_flag",
                  size, seq, ax)

fig1.tight_layout()
fig1.savefig("speedup_flag.png", dpi=150, bbox_inches="tight")
print("Saved: speedup_flag.png")

# ── Figure 2: num_cores variants ─────────────────────────────────────────────
fig2, axes2 = plt.subplots(1, 3, figsize=(18, 5))
fig2.suptitle("Speedup over Sequential Using Num_Cores (min time) — async vs bulk_async",
              fontsize=14, fontweight="bold", y=1.02)

for ax, size in zip(axes2, SIZES):
    speedup_panel(async_num_cores, bulk_num_cores,
                  "async_num_cores", "bulk_async_num_cores",
                  size, seq, ax)

fig2.tight_layout()
fig2.savefig("speedup_num_cores.png", dpi=150, bbox_inches="tight")
print("Saved: speedup_num_cores.png")

print("Done.")
