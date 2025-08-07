# Implementation Plan

- [x] 1. Set up project structure and create basic test framework

  - Review current main.cpp structure and identify all components that need modularization
  - Create directory structure following the modular design
  - Implement basic TestRunner class to verify Instance1 functionality with current code
  - Create Makefile or CMakeLists.txt for build system
  - _Requirements: 7.1, 7.4_

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

- [ ] 5. Implement research-based initial solution generation (Highest Impact)

  - [-] 5.1 Implement the 5-step feasible initial solution heuristic




    - Create InitialSolutionGenerator class based on the paper's algorithm
    - Implement Step 1: Assign annual leave (PreAssignedDaysOff) by blocking those cells
    - Implement Step 2: Cover weekend requirements ensuring each nurse has ≥2 weekends off
    - Implement Step 3: Assign shifts for first 4 days considering previous schedule constraints
    - Implement Step 4: Iterate day-by-day for remaining horizon, assigning shifts to meet coverage
    - Implement Step 5: Adjust working hours by adding shifts to meet minimum hour requirements
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6_

  - [ ] 5.2 Replace random initialization with feasible initial solution

    - Replace current schedule.randomize() calls with InitialSolutionGenerator::generateFeasibleSolution()
    - Verify that generated initial solutions are feasible or very close to feasible
    - Measure improvement in initial solution quality compared to random initialization
    - Test with multiple instances to ensure robustness
    - _Requirements: 3.1, 7.5_

  - [ ] 5.3 Validate initial solution quality improvements

    - Create tests to measure feasibility percentage of initial solutions
    - Compare initial solution scores with random initialization baseline
    - Verify that SA converges faster from better initial solutions
    - Document the improvement in number of iterations to reach first feasible solution
    - _Requirements: 7.5, 7.1_

- [ ] 6. Implement the 8 neighborhood structures from research (Core Algorithm)

  - [ ] 6.1 Implement Merge/Split moves (NS1, NS2)

    - Create MergeMove class: convert M+E shifts to L when preferences justify it
    - Create SplitMove class: convert L shift to M+E when preferences are better
    - Implement logic to find valid merge/split opportunities based on shift preferences
    - Add feasibility checking to ensure moves don't violate hard constraints
    - Write unit tests for merge/split move generation and application
    - _Requirements: 4.1, 4.6_

  - [ ] 6.2 Implement Block Swap move (NS3)

    - Create BlockSwapMove class for cross-exchange between 2 employees and 2 days
    - Implement logic: employee1 swaps shifts between day1 and day2, employee2 does same
    - Add validation to ensure block swap maintains feasibility
    - Write unit tests for block swap move generation and application
    - _Requirements: 4.2, 4.6_

  - [ ] 6.3 Implement 3-Way-Swap move (NS8)

    - Create ThreeWaySwapMove class for cyclic exchange of 3 employees on same day
    - Implement logic: shift of emp1 → emp2, shift of emp2 → emp3, shift of emp3 → emp1
    - Add validation to ensure 3-way swap maintains coverage and feasibility
    - Write unit tests for 3-way swap move generation and application
    - _Requirements: 4.3, 4.6_

  - [ ] 6.4 Implement Combined Neighborhood Structure (CNS)

    - Create CombinedNeighborhood class that tries all 8 move types per iteration
    - Implement strategy to generate one move of each type and select the best one
    - Replace current random move selection with CNS best-move selection
    - Verify that CNS produces better results than individual move types
    - _Requirements: 4.4, 4.6_

- [ ] 7. Implement true incremental evaluation (Critical Prerequisite)

  - [ ] 7.1 Refactor IncrementalEvaluator for true incremental computation

    - Review current IncrementalEvaluator and identify why it's not truly incremental
    - Redesign to maintain separate scores for each employee and each day
    - Implement delta calculation that only recalculates affected constraints
    - Create state management for constraint contributions by employee and day
    - _Requirements: 5.1, 5.2, 5.6_

  - [ ] 7.2 Implement constraint-specific delta calculations

    - Create delta calculation methods for each constraint type (max shifts, consecutive work, etc.)
    - Implement employee-specific delta calculation for constraints affecting individual employees
    - Implement day-specific delta calculation for coverage constraints
    - Ensure delta calculations produce exactly the same results as full evaluation
    - _Requirements: 5.2, 5.5_

  - [ ] 7.3 Add move application and reversion with state updates

    - Implement applyMove() that updates incremental state efficiently
    - Implement revertMove() that can undo changes without full recalculation
    - Add validation mode that compares incremental results with full evaluation
    - Measure performance improvement: should be 10-50x faster than full evaluation
    - _Requirements: 5.3, 5.4, 5.6_

  - [ ] 7.4 Integrate incremental evaluator with new neighborhood structures

    - Update all 8 move types to work with the new incremental evaluator
    - Ensure CNS can efficiently evaluate all move types using incremental computation
    - Verify that multiple move evaluations per iteration don't cause performance bottlenecks
    - Test that incremental evaluation enables practical use of all 8 neighborhood structures
    - _Requirements: 5.1, 5.6, 4.4_

- [ ] 8. Implement adaptive parameter tuning based on problem size

  - [ ] 8.1 Create ParameterTuner class with research-based parameter sets

    - Implement ParameterTuner class with predefined parameter sets for different problem sizes
    - Define small problem parameters (1-10 employees) based on paper's Table 5
    - Define medium problem parameters (11-30 employees) based on paper's recommendations
    - Define large problem parameters (31-60 employees) based on paper's findings
    - _Requirements: 6.1, 6.2, 6.3, 6.4_

  - [ ] 8.2 Integrate automatic parameter selection into SimulatedAnnealing

    - Modify SimulatedAnnealing constructor to automatically detect problem size
    - Implement logic to select appropriate parameter set based on num_employees and horizon
    - Add logging to document which parameter set is being used for each instance
    - Verify that different instances use appropriate parameter sets automatically
    - _Requirements: 6.1, 6.5, 6.6_

  - [ ] 8.3 Validate parameter tuning effectiveness

    - Run comparative tests using fixed parameters vs adaptive parameters
    - Measure performance improvement across different instance sizes
    - Document which parameter set works best for each type of problem
    - Verify that adaptive tuning improves average performance across all instances
    - _Requirements: 6.6, 7.4_

- [ ] 9. Implement comprehensive testing and validation system

  - [ ] 9.1 Create tests for initial solution quality

    - Implement tests that measure feasibility percentage of generated initial solutions
    - Create baseline comparison with random initialization
    - Add tests to verify that initial solutions satisfy most hard constraints
    - Measure improvement in SA convergence speed from better initial solutions
    - _Requirements: 7.1, 7.5_

  - [ ] 9.2 Create tests for neighborhood structure effectiveness

    - Implement tests that validate each of the 8 move types works correctly
    - Create statistical tests to verify CNS outperforms individual move types
    - Add tests to measure solution quality improvement from enhanced neighborhoods
    - Verify that new moves maintain feasibility and improve objective function
    - _Requirements: 7.2, 7.6_

  - [ ] 9.3 Create performance benchmarking and regression testing

    - Implement comprehensive benchmarking comparing old vs new implementation
    - Add automated tests that detect performance regressions
    - Create tests that verify incremental evaluation produces identical results to full evaluation
    - Measure and document overall performance improvements (target: 60-80% faster)
    - _Requirements: 7.3, 7.4_

- [ ] 10. Final integration and validation

  - [ ] 10.1 Integration testing with all improvements combined

    - Test complete system with initial solution + 8 neighborhoods + incremental evaluation + adaptive parameters
    - Verify that all improvements work together without conflicts
    - Run comprehensive tests on all available instances (Instance1, Instance2, Instance3, etc.)
    - Validate that combined system produces consistently better results than original
    - _Requirements: 7.1, 7.2, 7.3, 7.4_

  - [ ] 10.2 Create final performance report and documentation

    - Document performance improvements achieved by each optimization
    - Create comparison report showing before/after metrics for solution quality and execution time
    - Document lessons learned from implementing research-based improvements
    - Create user guide for running the optimized NSP solver
    - _Requirements: 7.4, 7.6_
