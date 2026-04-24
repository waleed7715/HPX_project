import pandas as pd

def process_csv(file_path):
    df = pd.read_csv(file_path)

    # Convert size to integer (removes scientific notation like 1E+09)
    df["size"] = df["size"].astype(float).astype(int)

    # Group ONLY by size and threads (aggregate across runs)
    grouped = (
        df.groupby(["size", "threads"])["duration_us"]
        .agg(mean="mean", min="min")
        .reset_index()
    )

    # Formatting
    grouped["mean"] = grouped["mean"].round(1)

    # Sort properly
    grouped = grouped.sort_values(["size", "threads"])

    return grouped


def save_output(df, output_file="openmp_data_par.csv"):
    df.to_csv(output_file, index=False)
    print(f"Saved {output_file}")


# Run
df = process_csv("results_omp.csv")
save_output(df)
