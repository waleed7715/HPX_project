#!/bin/bash

RUNS=20
INPUT_SIZES=(100000 10000000 1000000000)

PSTL_OUT="results_tbb_pstl.csv"

echo "size,threads,result_size,duration_us,run" > $PSTL_OUT

for INPUT_SIZE in "${INPUT_SIZES[@]}"; do
    echo "=== Input size: $INPUT_SIZE ==="

    echo "Running TBB PSTL (parallel scan)..."
    for run in $(seq 1 $RUNS); do
        ./build/copy_if_std_par $INPUT_SIZE 0 | while IFS= read -r line; do
            echo "$line,$run"
        done >> $PSTL_OUT
    done
done

echo "Done."
echo "  PSTL -> $PSTL_OUT"
