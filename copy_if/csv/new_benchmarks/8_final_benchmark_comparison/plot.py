import pandas as pd
import matplotlib.pyplot as plt

# --- Data Loading ---
seq = pd.read_csv("seq.csv")
seq = seq.set_index("size")["min"].to_dict()

def load(path):
    df = pd.read_csv(path)
    df["size"] = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
    return df.set_index(["size", "threads"])["min"]

# Baseline & Optimized
old_data = [load(f"old_using_num_chunks_{c}.csv") for c in ["1", "2", "default"]]
new_data = [load(f"new_using_num_chunks_{c}.csv") for c in ["1", "2", "default"]]

# Competitors
tbb_par      = load("tbb_data_par.csv")
openmp_par   = load("openmp_data_par.csv")
taskflow_par = load("taskflow_data_par.csv")

def get_time(key, size, t):
    """Helper to fetch the execution time based on framework key."""
    try:
        if key == "new":
            return min(s.loc[(size, t)] for s in new_data if (size, t) in s.index)
        elif key == "old":
            return min(s.loc[(size, t)] for s in old_data if (size, t) in s.index)
        elif key == "tbb": return tbb_par.loc[(size, t)]
        elif key == "omp": return openmp_par.loc[(size, t)]
        elif key == "tf":  return taskflow_par.loc[(size, t)]
    except (KeyError, ValueError):
        return None
    return None

# --- Configuration ---
threads = [1, 2, 4, 8, 12, 16, 24]
sizes = [100_000, 10_000_000, 1_000_000_000]
size_labels = ["100k", "10M", "1B"]

series = [
    ("HPX (Optimized)", "new", "#0077BB", "-",  "o"),
    ("HPX (Baseline)",  "old", "#CC3311", "--", "o"),
    ("TBB (PSTL)",      "tbb", "#009988", "-.", "o"),
    ("OpenMP",          "omp", "#EE7733", "-.", "o"),
    ("Taskflow",        "tf",  "#AA3377", "-.", "o"),
]

# --- Plotting & Reporting ---
print(f"{'Size':<10} | {'Framework':<18} | {'Speedup (24 Threads)':<15}")
print("-" * 50)

for size, label in zip(sizes, size_labels):
    fig, ax = plt.subplots(figsize=(10, 6))
    base = seq[size]

    for name, key, color, ls, marker in series:
        # Calculate speedups for all thread counts
        speedups = []
        for t in threads:
            t_val = get_time(key, size, t)
            speedups.append(base / t_val if t_val else None)

        # Plot the data
        ax.plot(threads, speedups, marker=marker, color=color, 
                linestyle=ls, linewidth=2.5, markersize=5, label=name)

        # Print the speedup for 24 threads (the last item in our thread list)
        final_speedup = speedups[-1]
        if final_speedup is not None:
            print(f"{label:<10} | {name:<18} | {final_speedup:>14.2f}x")
        else:
            print(f"{label:<10} | {name:<18} | {'N/A':>15}")

    # Plot styling
    ax.axhline(y=1.0, color="black", lw=1, ls=":", label="Sequential (1×)")
    ax.set_title(f"Cross-Framework Speedup (Size: {label})", fontsize=13, fontweight="bold")
    ax.set_xlabel("Number of Threads", fontsize=11)
    ax.set_ylabel("Speedup vs Sequential (×)", fontsize=11)
    ax.set_xticks(threads)
    ax.grid(True, alpha=0.3)
    ax.set_ylim(bottom=0)
    ax.legend(fontsize=10)
    
    fig.tight_layout()
    plt.savefig(f"speedup_flag_{size}.png", dpi=150)
    plt.close(fig) # Close to save memory if running in a loop

print("-" * 50)
print("Plots saved successfully.")
