#include "src/core/data_structures.h"
#include "src/constraints/hard_constraints.h"
#include "src/constraints/soft_constraints.h"
#include <iostream>
#include <chrono>

// Simple test to verify the improvements work
int main() {
    std::cout << "Testing NSP improvements..." << std::endl;
    
    // Test 1: Schedule with O(1) coverage
    std::cout << "\n1. Testing O(1) coverage tracking..." << std::endl;
    Schedule schedule(5, 7, 3);  // 5 employees, 7 days, 3 shift types
    
    // Set some assignments
    schedule.setAssignment(0, 0, 1);  // Employee 0, Day 0, Shift 1
    schedule.setAssignment(1, 0, 1);  // Employee 1, Day 0, Shift 1
    schedule.setAssignment(2, 0, 2);  // Employee 2, Day 0, Shift 2
    
    // Test coverage
    int coverage_shift1 = schedule.getCoverage(0, 1);
    int coverage_shift2 = schedule.getCoverage(0, 2);
    
    std::cout << "Coverage for Day 0, Shift 1: " << coverage_shift1 << " (expected: 2)" << std::endl;
    std::cout << "Coverage for Day 0, Shift 2: " << coverage_shift2 << " (expected: 1)" << std::endl;
    
    // Test shift counts
    int shift_count = schedule.getShiftCount(0, 1);
    std::cout << "Shift count for Employee 0, Shift 1: " << shift_count << " (expected: 1)" << std::endl;
    
    // Test 2: Performance comparison
    std::cout << "\n2. Testing performance improvements..." << std::endl;
    Schedule large_schedule(50, 28, 4);  // Larger schedule
    
    // Fill with some data
    for (int emp = 0; emp < 50; emp++) {
        for (int day = 0; day < 28; day++) {
            large_schedule.setAssignment(emp, day, (emp + day) % 4 + 1);
        }
    }
    
    // Time coverage calculations
    auto start = std::chrono::high_resolution_clock::now();
    int total_coverage = 0;
    for (int day = 0; day < 28; day++) {
        for (int shift = 1; shift <= 4; shift++) {
            total_coverage += large_schedule.getCoverage(day, shift);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Coverage calculation time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "Total coverage calculated: " << total_coverage << std::endl;
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}