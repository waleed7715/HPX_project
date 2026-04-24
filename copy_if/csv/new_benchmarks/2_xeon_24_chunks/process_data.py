import pandas as pd

COLS = ["run", "input_size", "threads", "num_chunks", "copied_elements", "duration_us"]

def process_csv(file_path):
    df = pd.read_csv(file_path, skipinitialspace=True, header=0, names=COLS)

    grouped = (
        df.groupby(["input_size", "threads", "num_chunks", "copied_elements"])["duration_us"]
        .agg(mean="mean", min="min")
        .reset_index()
    )

    grouped["mean"] = grouped["mean"].round(1)
    return grouped

def print_table(title, df):
    print("HPX:")
    print(title)

    for num_chunks, chunk_group in df.groupby("num_chunks"):
        print(f"\n--- num_chunks = {num_chunks} ---")
        print("size,threads,num_chunks,copied_elements,mean,min")

        for _, row in chunk_group.iterrows():
            print(f"{row['input_size']},{row['threads']},{row['num_chunks']},{row['copied_elements']},{row['mean']},{int(row['min'])}")

flag_df  = process_csv("results_using_flag.csv")
cores_df = process_csv("results_using_num_cores.csv")

print_table("Using flag:", flag_df)
print()
print_table("Using num_cores:", cores_df)
