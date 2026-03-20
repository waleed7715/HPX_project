import pandas as pd

def process_csv(file_path):
    df = pd.read_csv(file_path)

    grouped = (
        df.groupby(["size", "threads"])["duration_us"]
        .agg(mean="mean", min="min")
        .reset_index()
    )

    grouped["mean"] = grouped["mean"].round(1)
    grouped = grouped.sort_values(["size", "threads"])

    return grouped

def print_table(title, df):
    print("HPX:")
    print(title)
    print("size,threads,mean,min")

    for _, row in df.iterrows():
        print(f"{row['size']},{row['threads']},{row['mean']},{int(row['min'])}")

flag_df = process_csv("results_tbb_pstl.csv")
print_table("Using flag:", flag_df)
