@echo off
echo Building NSP Optimization Project...

REM Create directories
if not exist build mkdir build
if not exist build\core mkdir build\core
if not exist build\tests mkdir build\tests
if not exist bin mkdir bin

echo Compiling core modules...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c src/core/data_structures.cpp -o build/core/data_structures.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c src/core/instance_parser.cpp -o build/core/instance_parser.o

echo Compiling tests...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c tests/test_runner.cpp -o build/tests/test_runner.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c tests/test_main.cpp -o build/tests/test_main.o

echo Linking test runner...
g++ -std=c++17 -Wall -Wextra -O2 -o bin/test_runner.exe build/core/data_structures.o build/core/instance_parser.o build/tests/test_runner.o build/tests/test_main.o

echo Compiling optimized main...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -o bin/nsp_optimized.exe main_optimized.cpp build/core/data_structures.o build/core/instance_parser.o

echo Compiling original for comparison...
g++ -std=c++17 -Wall -Wextra -O2 -o bin/nsp_original.exe main.cpp

echo Build complete!
echo.
echo Available executables:
echo   bin/test_runner.exe     - Run tests
echo   bin/nsp_optimized.exe   - Optimized version (in progress)
echo   bin/nsp_original.exe    - Original version