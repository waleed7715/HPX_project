import pandas as pd
import matplotlib.pyplot as plt

seq = pd.read_csv("seq.csv")
seq = seq.set_index("size")["min"].to_dict()

def load(path):
    df = pd.read_csv(path)
    df["size"] = df["size"].astype(int)
    df["threads"] = df["threads"].astype(int)
    return df.set_index(["size", "threads"])["min"]

old_c1  = load("old_using_num_chunks_1.csv")
old_c2  = load("old_using_num_chunks_2.csv")
old_def = load("old_using_num_chunks_default.csv")
new_c1  = load("new_using_num_chunks_1.csv")
new_c2  = load("new_using_num_chunks_2.csv")
new_def = load("new_using_num_chunks_default.csv")

threads = [1, 2, 4, 8, 12, 16, 24]
sizes   = [100_000, 10_000_000, 1_000_000_000]
size_labels = ["100,000", "10,000,000", "1,000,000,000"]

def best(size, t, *sources):
    times = []
    for s in sources:
        try: times.append(s.loc[(size, t)])
        except KeyError: pass
    return min(times) if times else None

for size, label in zip(sizes, size_labels):
    base = seq[size]

    old_sp = [base / best(size, t, old_c1, old_c2, old_def) for t in threads]
    new_sp = [base / best(size, t, new_c1, new_c2, new_def) for t in threads]

    y_max = max(max(old_sp), max(new_sp)) * 1.15

    fig, axes = plt.subplots(1, 2, figsize=(20, 5))
    fig.subplots_adjust(wspace=0.3)
    
    for col, (sp, title) in enumerate([
        (old_sp, f"Baseline HPX — n = {label}"),
        (new_sp, f"Optimized HPX — n = {label}"),
    ]):
        ax = axes[col]

        ax.plot(threads, sp,
                marker="o", color="#0077BB",
                linewidth=2.5, markersize=5, label="HPX")

        ax.axhline(y=1.0, color="black", linewidth=1,
                   linestyle=":", label="Sequential (1×)")

        ax.set_title(title, fontsize=11, fontweight="bold")
        ax.set_xlabel("Number of Threads", fontsize=10)
        ax.set_ylabel("Speedup (×)", fontsize=10)
        ax.set_xticks(threads)
        ax.set_ylim(0, y_max)
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=9)

    fig.savefig(f"comparison_{size}.png", dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved comparison_{size}.png")
