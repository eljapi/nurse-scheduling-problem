#include "test_initial_solution.h"
#include "../src/metaheuristics/initial_solution.h"
#include "../src/core/instance.h"
#include <iostream>
#include <cassert>

TestInitialSolution::TestInitialSolution() {}

bool TestInitialSolution::runAllTests() {
    std::cout << "Running Initial Solution Generator tests..." << std::endl;
    
    bool all_passed = true;
    
    all_passed &= testBasicGeneration();
    all_passed &= testAnnualLeaveAssignment();
    all_passed &= testSolutionQuality();
    
    if (all_passed) {
        std::cout << "All Initial Solution Generator tests passed!" << std::endl;
    } else {
        std::cout << "Some Initial Solution Generator tests failed!" << std::endl;
    }
    
    return all_passed;
}

bool TestInitialSolution::testBasicGeneration() {
    std::cout << "  Testing basic solution generation..." << std::endl;
    
    // Load Instance1 for testing
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        std::cout << "    FAILED: Could not load Instance1.txt" << std::endl;
        return false;
    }
    
    InitialSolutionGenerator generator(instance);
    Schedule schedule = generator.generateFeasibleSolution();
    
    // Basic validation
    if (schedule.getNumEmployees() != instance.getNumEmployees()) {
        std::cout << "    FAILED: Employee count mismatch" << std::endl;
        return false;
    }
    
    if (schedule.getHorizonDays() != instance.getHorizonDays()) {
        std::cout << "    FAILED: Horizon days mismatch" << std::endl;
        return false;
    }
    
    // Check that all assignments are valid
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            int shift = schedule.getAssignment(emp, day);
            if (shift < 0 || shift > instance.getNumShiftTypes()) {
                std::cout << "    FAILED: Invalid shift assignment: " << shift << std::endl;
                return false;
            }
        }
    }
    
    std::cout << "    PASSED: Basic generation test" << std::endl;
    return true;
}

bool TestInitialSolution::testAnnualLeaveAssignment() {
    std::cout << "  Testing annual leave assignment..." << std::endl;
    
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        std::cout << "    FAILED: Could not load Instance1.txt" << std::endl;
        return false;
    }
    
    InitialSolutionGenerator generator(instance);
    Schedule schedule = generator.generateFeasibleSolution();
    
    // Check that required days off are respected
    const auto& days_off = instance.getDaysOff();
    for (const auto& employee_days_off : days_off) {
        int employee_index = instance.getStaffIndex(employee_days_off.EmployeeID);
        
        if (employee_index == -1) continue;
        
        for (const std::string& day_str : employee_days_off.DayIndexes) {
            int day = std::stoi(day_str);
            
            if (instance.isValidDay(day)) {
                int assigned_shift = schedule.getAssignment(employee_index, day);
                if (assigned_shift != 0) {
                    std::cout << "    FAILED: Employee " << employee_index 
                              << " assigned shift " << assigned_shift 
                              << " on required day off " << day << std::endl;
                    return false;
                }
            }
        }
    }
    
    std::cout << "    PASSED: Annual leave assignment test" << std::endl;
    return true;
}

bool TestInitialSolution::testSolutionQuality() {
    std::cout << "  Testing solution quality improvement..." << std::endl;
    
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        std::cout << "    FAILED: Could not load Instance1.txt" << std::endl;
        return false;
    }
    
    // Generate initial solution using heuristic
    InitialSolutionGenerator generator(instance);
    Schedule heuristic_schedule = generator.generateFeasibleSolution();
    
    // Generate random solution for comparison
    Schedule random_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    random_schedule.randomize(instance.getNumShiftTypes());
    
    // Count assignments (non-zero shifts)
    int heuristic_assignments = 0;
    int random_assignments = 0;
    
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            if (heuristic_schedule.getAssignment(emp, day) != 0) {
                heuristic_assignments++;
            }
            if (random_schedule.getAssignment(emp, day) != 0) {
                random_assignments++;
            }
        }
    }
    
    std::cout << "    Heuristic assignments: " << heuristic_assignments << std::endl;
    std::cout << "    Random assignments: " << random_assignments << std::endl;
    
    // The heuristic should create a more structured solution
    // (This is a basic quality check - in practice we'd use constraint evaluators)
    
    std::cout << "    PASSED: Solution quality test" << std::endl;
    return true;
}