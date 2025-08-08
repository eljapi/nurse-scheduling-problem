#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include "src/constraints/constraint_evaluator.h"
#include "src/metaheuristics/neighborhood.h"
#include "src/constraints/incremental_evaluator.h"
#include <iostream>

int main() {
    std::cout << "Testing movement and evaluation..." << std::endl;
    
    // Create a simple test instance
    Schedule schedule(3, 5, 2);  // 3 employees, 5 days, 2 shift types
    
    // Set some initial assignments
    schedule.setAssignment(0, 0, 1);  // Employee 0, Day 0, Shift 1
    schedule.setAssignment(1, 1, 2);  // Employee 1, Day 1, Shift 2
    schedule.setAssignment(2, 2, 1);  // Employee 2, Day 2, Shift 1
    
    std::cout << "Initial schedule:" << std::endl;
    for (int emp = 0; emp < 3; emp++) {
        std::cout << "Employee " << emp << ": ";
        for (int day = 0; day < 5; day++) {
            std::cout << schedule.getAssignment(emp, day) << " ";
        }
        std::cout << std::endl;
    }
    
    // Test a simple change move
    std::cout << "\nTesting change move: Employee 0, Day 1, from 0 to 2" << std::endl;
    int old_shift = schedule.getAssignment(0, 1);
    schedule.setAssignment(0, 1, 2);
    int new_shift = schedule.getAssignment(0, 1);
    
    std::cout << "Changed from " << old_shift << " to " << new_shift << std::endl;
    
    // Test swap
    std::cout << "\nTesting swap: Employee 0 Day 0 <-> Employee 1 Day 1" << std::endl;
    int shift_0_0 = schedule.getAssignment(0, 0);
    int shift_1_1 = schedule.getAssignment(1, 1);
    
    std::cout << "Before swap: Emp0Day0=" << shift_0_0 << ", Emp1Day1=" << shift_1_1 << std::endl;
    
    schedule.swapAssignments(0, 0, 1, 1);
    
    int new_shift_0_0 = schedule.getAssignment(0, 0);
    int new_shift_1_1 = schedule.getAssignment(1, 1);
    
    std::cout << "After swap: Emp0Day0=" << new_shift_0_0 << ", Emp1Day1=" << new_shift_1_1 << std::endl;
    
    std::cout << "\nFinal schedule:" << std::endl;
    for (int emp = 0; emp < 3; emp++) {
        std::cout << "Employee " << emp << ": ";
        for (int day = 0; day < 5; day++) {
            std::cout << schedule.getAssignment(emp, day) << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nMovement test completed!" << std::endl;
    return 0;
}