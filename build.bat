@echo off
echo Building NSP Optimization Project...

REM Create directories
if not exist build mkdir build
if not exist build\core mkdir build\core
if not exist build\constraints mkdir build\constraints
if not exist build\tests mkdir build\tests
if not exist build\utils mkdir build\utils
if not exist bin mkdir bin
if not exist src\constraints mkdir src\constraints

echo Compiling core modules...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c src/core/data_structures.cpp -o build/core/data_structures.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c src/core/instance_parser.cpp -o build/core/instance_parser.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c src/core/instance.cpp -o build/core/instance.o

echo Compiling constraints modules...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c src/constraints/hard_constraints.cpp -o build/constraints/hard_constraints.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c src/constraints/soft_constraints.cpp -o build/constraints/soft_constraints.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c src/constraints/constraint_evaluator.cpp -o build/constraints/constraint_evaluator.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/constraints/incremental_evaluator.cpp -o build/incremental_evaluator.o

echo Compiling metaheuristics modules...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/metaheuristics/neighborhood.cpp -o build/neighborhood.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/metaheuristics/simulated_annealing.cpp -o build/simulated_annealing.o

echo Compiling tests...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c tests/test_runner.cpp -o build/tests/test_runner.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c tests/test_hard_constraints.cpp -o build/tests/test_hard_constraints.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c tests/test_soft_constraints.cpp -o build/tests/test_soft_constraints.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c tests/test_solution_validator.cpp -o build/tests/test_solution_validator.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -c tests/test_instance10_validator.cpp -o build/tests/test_instance10_validator.o
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -c tests/test_main.cpp -o build/tests/test_main.o

echo Linking test runner...
g++ -std=c++17 -Wall -Wextra -O2 -o bin/test_runner.exe build/core/data_structures.o build/core/instance_parser.o build/core/instance.o build/constraints/hard_constraints.o build/constraints/soft_constraints.o build/constraints/constraint_evaluator.o build/tests/test_runner.o build/tests/test_hard_constraints.o build/tests/test_soft_constraints.o build/tests/test_solution_validator.o build/tests/test_instance10_validator.o build/tests/test_main.o

echo Compiling optimized main...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -o bin/nsp_optimized.exe main_optimized.cpp build/core/data_structures.o build/core/instance_parser.o build/core/instance.o

echo Compiling utils modules...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -c src/utils/random.cpp -o build/utils/random.o

echo Compiling refactored main...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o bin/nsp_refactored.exe main_refactored.cpp build/core/data_structures.o build/core/instance_parser.o build/core/instance.o build/constraints/hard_constraints.o build/constraints/soft_constraints.o build/constraints/constraint_evaluator.o build/neighborhood.o build/simulated_annealing.o build/incremental_evaluator.o build/utils/random.o

echo Compiling original for comparison...
g++ -std=c++17 -Wall -Wextra -O2 -o bin/nsp_original.exe main.cpp

echo Compiling hard constraints demo...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -o bin/demo_hard_constraints.exe demo_hard_constraints.cpp build/core/data_structures.o build/core/instance_parser.o build/core/instance.o build/constraints/hard_constraints.o build/constraints/soft_constraints.o

echo Compiling Instance1 validation test...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -o bin/test_instance1.exe test_instance1_comparison.cpp build/core/data_structures.o build/core/instance_parser.o build/core/instance.o build/constraints/hard_constraints.o build/constraints/soft_constraints.o

echo Compiling constraint debug tool...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc/core -Isrc/constraints -o bin/debug_constraints.exe debug_constraints.cpp build/core/data_structures.o build/core/instance_parser.o build/core/instance.o build/constraints/hard_constraints.o build/constraints/soft_constraints.o

echo Compiling soft constraints demo...
g++ -std=c++17 -Wall -Wextra -O2 -Isrc -o bin/demo_soft_constraints.exe demo_soft_constraints.cpp build/core/data_structures.o build/core/instance_parser.o build/core/instance.o build/constraints/soft_constraints.o

echo Build complete!
echo.
echo Available executables:
echo   bin/test_runner.exe     - Run tests
echo   bin/nsp_optimized.exe   - Optimized version (in progress)
echo   bin/nsp_refactored.exe  - Refactored version using new data structures
echo   bin/nsp_original.exe    - Original version
echo   bin/demo_hard_constraints.exe - Demo of HardConstraints class
echo   bin/test_instance1.exe - Instance1 validation test
echo   bin/debug_constraints.exe - Debug constraint functions
echo   bin/demo_soft_constraints.exe - Demo of SoftConstraints class
