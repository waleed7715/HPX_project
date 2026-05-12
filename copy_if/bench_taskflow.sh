#!/usr/bin/env bash

echo "Starting Taskflow copy_if benchmark"

BINARY="build/copy_if_taskflow"
OUTPUT="results_taskflow.csv"
RUNS=20

echo "run,input_size,threads,num_chunks,copied_elements,duration_us" > "$OUTPUT"

for t in 1 2 4 8 12 16 24; do
    for s in 100000 10000000 1000000000; do
        for ((r=1; r<=RUNS; r++)); do
            echo "Running: threads=$t size=$s run=$r/$RUNS"

            result=$(OMP_PROC_BIND=close "$BINARY" --threads="$t" --input_size="$s" --runs=1 2>/dev/null)

            IFS=',' read -r input_sz threads_ num_chunks_ copied duration <<< "$result"
            echo "$r,$input_sz,$threads_,$num_chunks_,$copied,$duration" >> "$OUTPUT"
        done
    done
done

echo "Done — results written to $OUTPUT"
