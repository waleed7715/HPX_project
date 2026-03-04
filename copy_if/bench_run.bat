@echo off
setlocal enabledelayedexpansion

REM ── Benchmark 1: num_cores via --hpx_threads ───────────────────────────────

echo Starting benchmark using num_cores via flag --hpx_threads

set BINARY=build\release\copy_if_hpx.exe
set OUTPUT=results_using_num_cores.csv
set RUNS=10

echo run,input_size,threads,copied_elements,duration_us > %OUTPUT%

for %%t in (1 2 4 8 16) do (
    for %%s in (100000 10000000 1000000000) do (
        for /l %%r in (1,1,%RUNS%) do (
            echo Running: threads=%%t size=%%s run=%%r

            for /f "delims=" %%A in ('%BINARY% --hpx_threads=%%t --input_size=%%s 2^>nul') do (
                set result=%%A

                for /f "tokens=1-4 delims=," %%a in ("!result!") do (
                    set input_sz=%%a
                    set threads_=%%b
                    set copied=%%c
                    set duration=%%d

                    echo %%r,!input_sz!,!threads_!,!copied!,!duration! >> %OUTPUT%
                )
            )
        )
    )
)

echo Done — writing finished to %OUTPUT%

REM ── Benchmark 2: --hpx:threads ─────────────────────────────────────────────

echo Starting benchmark using flag --hpx:threads

set BINARY=build\release\copy_if_hpx_clean.exe
set OUTPUT=results_using_flag.csv
set RUNS=10

echo run,input_size,threads,copied_elements,duration_us > %OUTPUT%

for %%t in (1 2 4 8 16) do (
    for %%s in (100000 10000000 1000000000) do (
        for /l %%r in (1,1,%RUNS%) do (
            echo Running: threads=%%t size=%%s run=%%r/%RUNS%

            for /f "delims=" %%A in ('%BINARY% --hpx:threads=%%t --input_size=%%s 2^>nul') do (
                set result=%%A

                for /f "tokens=1-4 delims=," %%a in ("!result!") do (
                    set input_sz=%%a
                    set threads_=%%b
                    set copied=%%c
                    set duration=%%d

                    echo %%r,!input_sz!,!threads_!,!copied!,!duration! >> %OUTPUT%
                )
            )
        )
    )
)

echo Done — writing finished to %OUTPUT%

endlocal
