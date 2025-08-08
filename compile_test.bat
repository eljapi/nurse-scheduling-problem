@echo off
echo Compiling test improvements...

g++ -std=c++17 -I. -O2 ^
    test_improvements.cpp ^
    src/core/data_structures.cpp ^
    -o test_improvements.exe

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful!
    echo Running test...
    test_improvements.exe
) else (
    echo Compilation failed!
)

pause