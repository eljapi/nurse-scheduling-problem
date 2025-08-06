# NSP Optimization Project

This project is a step-by-step optimization of a Nurse Scheduling Problem (NSP) implementation, transforming a monolithic C++ solution into a modular, optimized system with eventual CUDA support.

## Project Structure

```
nsp/
├── src/
│   ├── core/                    # Core data structures and parsing
│   │   ├── data_structures.h/cpp    # Schedule and data structures
│   │   └── instance_parser.h/cpp    # NSP instance file parser
│   ├── constraints/             # Constraint evaluation (future)
│   ├── metaheuristics/          # Optimization algorithms (future)
│   └── utils/                   # Utility functions (future)
├── tests/
│   ├── test_runner.h/cpp        # Test framework
│   └── test_main.cpp            # Test entry point
├── bin/                         # Compiled executables
├── build/                       # Build artifacts
├── nsp_instancias/             # Test instances
├── main.cpp                     # Original implementation
├── main_optimized.cpp          # New modular implementation
├── build.bat                   # Windows build script
└── Makefile                    # Unix build script
```

## Building

### Windows
```bash
./build.bat
```

### Unix/Linux
```bash
make
```

## Running

### Test the optimized version
```bash
./bin/nsp_optimized.exe Instance1.txt 1000
```

### Test the original version (for comparison)
```bash
./bin/nsp_original.exe Instance1.txt 1000
```

### Run tests
```bash
./bin/test_runner.exe
```

## Current Status

✅ **Phase 1: Project Structure and Basic Testing** (Completed)
- Modular project structure created
- Basic data structures implemented (Schedule, Staff, Shift, etc.)
- Instance parser working correctly
- Test framework operational
- Successfully parses and processes Instance1.txt

🔄 **Phase 2: Data Structure Optimization** (In Progress)
- Extract Instance class with optimized data loading
- Create Schedule class for solution representation
- Refactor main.cpp to use new structures

⏳ **Phase 3: Constraint System** (Planned)
- Extract hard and soft constraint evaluation
- Implement unified constraint evaluator

⏳ **Phase 4: Metaheuristic Optimization** (Planned)
- Improve Simulated Annealing implementation
- Add support for multiple instances

⏳ **Phase 5: CUDA Preparation** (Planned)
- Prepare code structure for GPU acceleration

## Key Features

- **Incremental Development**: Each phase maintains compatibility with existing functionality
- **Test-Driven**: Comprehensive testing ensures no regressions
- **Performance Focused**: Optimizations target both memory usage and execution speed
- **CUDA Ready**: Architecture designed for eventual GPU acceleration

## Testing

The project includes automated testing to ensure each optimization maintains correctness:

- **Unit Tests**: Individual component testing
- **Integration Tests**: End-to-end instance processing
- **Regression Tests**: Comparison with original implementation

## Original vs Optimized

The original `main.cpp` (1110+ lines) is being systematically refactored into:
- Modular, reusable components
- Optimized data structures
- Clean separation of concerns
- Maintainable, documented code

Both versions coexist and can be compared for correctness and performance.