import pandas as pd

def process_csv(file_path):
    df = pd.read_csv(file_path)

    grouped = (
        df.groupby(["input_size", "threads", "num_chunks"])["duration_us"]
        .agg(mean="mean", min="min")
        .reset_index()
    )

    grouped["mean"] = grouped["mean"].round(1)
    grouped = grouped.sort_values(["num_chunks", "input_size", "threads"])

    return grouped

def print_table(title, df):
    print("HPX:")
    print(title)
    print("size,threads,num_chunks,mean,min")

    for _, row in df.iterrows():
        print(f"{row['input_size']},{row['threads']},{int(row['num_chunks'])},{row['mean']},{int(row['min'])}")

flag_df = process_csv("results_using_flag.csv")
print_table("Using flag:", flag_df)
