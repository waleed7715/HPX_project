import pandas as pd

def process_csv(file_path):
    df = pd.read_csv(file_path)

    # Group by size and threads
    grouped = (
        df.groupby(["input_size", "threads"])["duration_us"]
        .agg(mean="mean", min="min")
        .reset_index()
    )

    # Round mean to 1 decimal like your example
    grouped["mean"] = grouped["mean"].round(1)

    return grouped

def print_table(title, df):
    print("HPX:")
    print(title)
    print("size,threads,mean,min")

    for _, row in df.iterrows():
        print(f"{row['input_size']},{row['threads']},{row['mean']},{int(row['min'])}")

# Process files
flag_df = process_csv("results_using_flag.csv")
cores_df = process_csv("results_using_num_cores.csv")

# Print results
print_table("Using flag:", flag_df)
print()
print_table("Using num_cores:", cores_df)
