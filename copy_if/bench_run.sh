#!/bin/bash

# ── Benchmark 1: num_cores via --hpx_threads ───────────────────────────────

echo "Starting benchmark using num_cores via flag --hpx_threads"

BINARY="build/release/copy_if_hpx"
OUTPUT="results_using_num_cores.csv"
RUNS=10

echo "run,input_size,threads,copied_elements,duration_us" > "$OUTPUT"

for t in 1 2 4 8 16; do
    for s in 100000 10000000 1000000000; do
        for ((r=1; r<=RUNS; r++)); do
            echo "Running: threads=$t size=$s run=$r"

            result=$("$BINARY" --hpx_threads="$t" --input_size="$s" 2>/dev/null)

            IFS=',' read -r input_sz threads_ copied duration <<< "$result"
            echo "$r,$input_sz,$threads_,$copied,$duration" >> "$OUTPUT"
        done
    done
done

echo "Done — writing finished to $OUTPUT"

# ── Benchmark 2: --hpx:threads ─────────────────────────────────────────────

echo "Starting benchmark using flag --hpx:threads"

BINARY="build/release/copy_if_hpx_clean"
OUTPUT="results_using_flag.csv"
RUNS=10

echo "run,input_size,threads,copied_elements,duration_us" > "$OUTPUT"

for t in 1 2 4 8 16; do
    for s in 100000 10000000 1000000000; do
        for ((r=1; r<=RUNS; r++)); do
            echo "Running: threads=$t size=$s run=$r/$RUNS"

            result=$("$BINARY" --hpx:threads="$t" --input_size="$s" 2>/dev/null)

            IFS=',' read -r input_sz threads_ copied duration <<< "$result"
            echo "$r,$input_sz,$threads_,$copied,$duration" >> "$OUTPUT"
        done
    done
done

echo "Done — writing finished to $OUTPUT"
