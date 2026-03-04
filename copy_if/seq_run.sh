#!/bin/bash

BINARY="build/release/copy_if_std_seq"
OUTPUT="results_std_seq.csv"
RUNS=30

echo "run,input_size,copied_elements,duration_us" > "$OUTPUT"

for ((r=1; r<=RUNS; r++)); do
    echo "Run $r/$RUNS"

    while IFS=',' read -r input_sz copied duration; do
        echo "$r,$input_sz,$copied,$duration" >> "$OUTPUT"
    done < <("$BINARY" 2>/dev/null)
done

echo "Done — writing finished to $OUTPUT"
