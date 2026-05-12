#!/bin/bash

echo "Starting benchmark using flag --hpx:threads"

BINARY="build/copy_if_hpx"
OUTPUT="results_using_flag.csv"
RUNS=20

echo "run,input_size,threads,num_chunks,copied_elements,duration_us" > "$OUTPUT"

for t in 1 2 4 8 12 16 24; do
    for s in 100000 10000000 1000000000; do
        for c in 0 1 2; do
            for ((r=1; r<=RUNS; r++)); do
                echo "Running: threads=$t size=$s num_chunks=$c run=$r/$RUNS"

                result=$("$BINARY" --hpx:threads="$t" --hpx:numa-sensitive=1 --input_size="$s" --num_chunks="$c" 2>/dev/null)

                IFS=',' read -r input_sz threads_ num_chunks_ copied duration <<< "$result"
                echo "$r,$input_sz,$threads_,$num_chunks_,$copied,$duration" >> "$OUTPUT"
            done
        done
    done
done

echo "Done — writing finished to $OUTPUT"
