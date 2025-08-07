# Implementation Plan

- [x] 1. Set up project structure and create basic test framework

  - Review current main.cpp structure and identify all components that need modularization
  - Create directory structure following the modular design
  - Implement basic TestRunner class to verify Instance1 functionality with current code
  - Create Makefile or CMakeLists.txt for build system
  - _Requirements: 1.3, 5.1, 5.4_

- [ ] 2. Extract and modularize core data structures

  - [x] 2.1 Create Instance class with optimized data loading

    - Review current parsing functions (addShift, addStaff, addDaysOff, etc.) in main.cpp
    - Extract instance parsing logic from main.cpp into InstanceParser class
    - Implement Instance class to hold all problem data with const getters
    - Write unit tests for instance loading and data access
    - _Requirements: 1.1, 1.3, 2.1_

  - [x] 2.2 Create Schedule class for solution representation

    - Review current matrix operations (MATRIX, aux_matrix, copyMatrix) in main.cpp
    - Implement Schedule class to replace raw 2D arrays with proper encapsulation
    - Add methods for assignment manipulation and copying
    - Create unit tests for schedule operations
    - _Requirements: 1.1, 1.3, 2.1_

  - [x] 2.3 Refactor main.cpp to use new data structures

    - Update main.cpp to use Instance and Schedule classes
    - Ensure Instance1 still runs correctly with new structure
    - Verify output matches original implementation
    - _Requirements: 1.1, 1.2, 1.4_

- [ ] 3. Extract and optimize constraint evaluation system

  - [x] 3.1 Create HardConstraints class

    - Review current hard constraint functions (sumOfShift, ShiftTimesSum, maxConsecutiveShifts, etc.)
    - Extract all hard constraint functions into HardConstraints class
    - Implement methods for each constraint type (max shifts, consecutive work, etc.)
    - Write unit tests for individual constraint evaluation
    - _Requirements: 1.3, 2.1, 5.2_

  - [x] 3.2 Create SoftConstraints class

    - Review current soft constraint functions (ShiftOnRequest, ShiftOffRequest, SectionCover)
    - Extract soft constraint functions into SoftConstraints class
    - Implement penalty calculation methods for requests and coverage
    - Write unit tests for soft constraint evaluation
    - _Requirements: 1.3, 2.1, 5.2_

  - [x] 3.3 Implement unified ConstraintEvaluator

    - Create ConstraintEvaluator class that combines hard and soft constraints
    - Replace scattered constraint calls in main loop with unified evaluation
    - Verify Instance1 produces same results with new evaluator
    - _Requirements: 1.1, 1.2, 2.1_

- [ ] 4. Optimize data structures for better performance

  - [ ] 4.1 Replace vectors with more efficient structures where appropriate

    - Review current vector usage in data structures (workers, turnos, libres, etc.)
    - Analyze memory usage patterns and identify optimization opportunities
    - Replace std::vector with std::array for fixed-size data
    - Implement memory-efficient packed representations where beneficial
    - _Requirements: 2.1, 2.2, 2.4_

  - [ ] 4.2 Implement cache-friendly data layouts

    - Reorganize data structures for better spatial locality
    - Use structure-of-arrays instead of array-of-structures where beneficial
    - Measure and verify memory access improvements
    - _Requirements: 2.1, 2.4_

  - [ ] 4.3 Add memory pre-allocation and reuse strategies
    - Implement object pooling for frequently created/destroyed objects
    - Pre-allocate working memory to avoid runtime allocations
    - Verify performance improvements with benchmarking
    - _Requirements: 2.1, 2.4_

- [ ] 5. Extract and improve the Simulated Annealing metaheuristic

  - [ ] 5.1 Create SimulatedAnnealing class

    - Review current SA implementation in main.cpp (temperature, cooling, acceptance function)
    - Extract SA logic from main.cpp into dedicated class
    - Implement configurable parameters (temperature, cooling rate, iterations)
    - Ensure Instance1 still solves correctly with extracted SA
    - _Requirements: 1.3, 3.2, 4.3_

  - [ ] 5.2 Implement improved neighborhood operators

    - Review current neighborhood exploration in main.cpp (nested loops changing shifts)
    - Create Neighborhood class with multiple move operators
    - Implement single-shift change, shift swap, and block moves
    - Add intelligent move selection based on constraint violations
    - _Requirements: 3.1, 3.2, 4.1_

  - [x] 5.3 Add incremental constraint evaluation




    - Implement IncrementalEvaluator for efficient move evaluation
    - Cache constraint contributions and update incrementally
    - Verify correctness and measure performance improvements
    - _Requirements: 2.4, 3.1, 3.2_

- [ ] 6. Enhance metaheuristic to solve multiple instances

  - [ ] 6.1 Improve cooling schedule and parameter tuning

    - Implement adaptive cooling schedules
    - Add parameter auto-tuning based on problem characteristics
    - Test with Instance1, Instance2, and Instance3
    - _Requirements: 3.1, 3.2, 3.3_

  - [ ] 6.2 Add diversification and intensification strategies





    - Implement restart mechanisms when search stagnates
    - Add tabu-like memory to avoid cycling
    - Implement local search intensification phases
    - _Requirements: 3.1, 3.2, 3.3_

  - [ ] 6.3 Validate solution quality across multiple instances
    - Run comprehensive tests on all available instances
    - Compare solution quality with original implementation
    - Document performance improvements and success rates
    - _Requirements: 3.1, 3.3, 5.1, 5.3_

- [ ] 7. Prepare code structure for CUDA integration

  - [ ] 7.1 Separate computation from control logic

    - Review current code to identify computationally intensive functions and side effects
    - Identify and isolate computationally intensive functions
    - Create pure functions without side effects for constraint evaluation
    - Implement data-parallel friendly algorithms
    - _Requirements: 4.1, 4.2, 4.3_

  - [ ] 7.2 Create GPU-friendly data structures

    - Design flat, contiguous memory layouts for GPU transfer
    - Implement serialization/deserialization for GPU data
    - Create mock CUDA interfaces for future implementation
    - _Requirements: 4.1, 4.2, 4.3_

  - [ ] 7.3 Document parallelization strategy
    - Identify specific functions and loops suitable for CUDA parallelization
    - Document memory access patterns and optimization opportunities
    - Create roadmap for CUDA kernel implementation
    - _Requirements: 4.2, 6.4_

- [ ] 8. Implement comprehensive testing and benchmarking system

  - [ ] 8.1 Create automated regression testing

    - Implement test suite that runs all instances automatically
    - Add performance regression detection
    - Create continuous integration setup for testing
    - _Requirements: 5.1, 5.2, 5.3, 5.4_

  - [ ] 8.2 Add performance profiling and benchmarking

    - Implement detailed timing for each optimization phase
    - Add memory usage profiling and reporting
    - Create performance comparison reports
    - _Requirements: 2.4, 5.4, 6.2_

  - [ ] 8.3 Validate correctness and solution quality
    - Implement solution feasibility checking
    - Add solution quality metrics and comparison tools
    - Verify all instances produce valid, high-quality solutions
    - _Requirements: 5.2, 5.3, 3.3_

- [ ] 9. Documentation and code cleanup

  - [ ] 9.1 Add comprehensive code documentation

    - Document all classes, methods, and key algorithms
    - Add inline comments explaining optimization rationale
    - Create API documentation for main interfaces
    - _Requirements: 6.1, 6.2_

  - [ ] 9.2 Create user guide and optimization report

    - Document how to build and run the optimized version
    - Create report comparing performance before and after optimizations
    - Document lessons learned and future improvement opportunities
    - _Requirements: 6.2, 6.3, 6.4_

  - [ ] 9.3 Final integration testing and validation
    - Run complete test suite on all instances
    - Verify all optimizations work together correctly
    - Create final performance benchmarks and quality metrics
    - _Requirements: 1.2, 2.2, 3.3, 5.1_
