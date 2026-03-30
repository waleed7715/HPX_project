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

def save_by_num_chunks(df):
    chunk_to_filename = {
        0: "using_num_chunks_default.csv",
        1: "using_num_chunks_1.csv",
        2: "using_num_chunks_2.csv",
    }

    for num_chunks, filename in chunk_to_filename.items():
        subset = df[df["num_chunks"] == num_chunks].copy()
        subset.columns = ["size", "threads", "num_chunks", "mean", "min"]
        subset.to_csv(filename, index=False)
        print(f"Saved {filename}")

flag_df = process_csv("results_using_flag.csv")
save_by_num_chunks(flag_df)
