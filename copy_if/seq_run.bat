@echo off
setlocal enabledelayedexpansion

set BINARY=build\release\copy_if_std_seq.exe
set OUTPUT=results_std_seq.csv
set RUNS=30

echo run,input_size,copied_elements,duration_us > %OUTPUT%

for /l %%r in (1,1,%RUNS%) do (
    echo Run %%r/%RUNS%

    REM Execute binary and capture each output line
    for /f "delims=" %%L in ('%BINARY% 2^>nul') do (
        
        REM Split CSV line into tokens
        for /f "tokens=1-3 delims=," %%a in ("%%L") do (
            set input_sz=%%a
            set copied=%%b
            set duration=%%c

            echo %%r,!input_sz!,!copied!,!duration! >> %OUTPUT%
        )
    )
)

echo Done — writing finished to %OUTPUT%

endlocal
