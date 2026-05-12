#!/bin/bash

BINARY="build/copy_if_std_seq"
OUTPUT="results_std_seq.csv"
RUNS=30
SIZES=(100000 10000000 1000000000)

echo "run,input_size,copied_elements,duration_us" > "$OUTPUT"

for ((r=1; r<=RUNS; r++)); do
    echo "Run $r/$RUNS"
    for size in "${SIZES[@]}"; do
        result=$("$BINARY" "$size" 2>/dev/null)
        echo "$r,$result" >> "$OUTPUT"
    done
done

echo "Done — results written to $OUTPUT"
