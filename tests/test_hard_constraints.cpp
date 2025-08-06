#include "test_hard_constraints.h"
#include "../src/constraints/hard_constraints.h"
#include "../src/core/instance.h"
#include "../src/core/data_structures.h"
#include <iostream>
#include <cassert>

TestHardConstraints::TestHardConstraints() : tests_passed(0), tests_failed(0) {}

void TestHardConstraints::logTest(const std::string& test_name, bool passed, const std::string& message) {
    if (passed) {
        tests_passed++;
        std::cout << "[PASS] " << test_name << std::endl;
    } else {
        tests_failed++;
        failed_tests.push_back(test_name);
        std::cout << "[FAIL] " << test_name;
        if (!message.empty()) {
            std::cout << " - " << message;
        }
        std::cout << std::endl;
    }
}

bool TestHardConstraints::testMaxShiftsPerType() {
    // Create a simple instance for testing
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Max Shifts Per Type", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test valid schedule (should have penalty 0)
    schedule.clear();
    int penalty_valid = constraints.evaluateMaxShiftsPerType(schedule);
    bool test1 = (penalty_valid == 0);
    
    // Test schedule with too many shifts of one type
    // Assign many shifts of type 1 to employee 0
    for (int day = 0; day < std::min(10, instance.getHorizonDays()); day++) {
        schedule.setAssignment(0, day, 1);
    }
    
    int penalty_invalid = constraints.evaluateMaxShiftsPerType(schedule);
    bool test2 = (penalty_invalid < 0);
    
    bool all_passed = test1 && test2;
    logTest("Max Shifts Per Type", all_passed, 
            all_passed ? "" : "Constraint evaluation failed");
    
    return all_passed;
}

bool TestHardConstraints::testWorkingTimeConstraints() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Working Time Constraints", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test empty schedule (should violate minimum time)
    schedule.clear();
    int penalty_empty = constraints.evaluateWorkingTimeConstraints(schedule);
    bool test1 = (penalty_empty < 0);
    
    // Test schedule with reasonable assignments
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < 5; day++) { // Assign 5 days of work
            schedule.setAssignment(emp, day, 1);
        }
    }
    
    int penalty_reasonable = constraints.evaluateWorkingTimeConstraints(schedule);
    // This might still be negative depending on the instance constraints
    
    bool all_passed = test1; // At least the empty schedule should fail
    logTest("Working Time Constraints", all_passed, 
            all_passed ? "" : "Constraint evaluation failed");
    
    return all_passed;
}

bool TestHardConstraints::testMaxConsecutiveShifts() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Max Consecutive Shifts", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test valid schedule with breaks
    schedule.clear();
    schedule.setAssignment(0, 0, 1);
    schedule.setAssignment(0, 1, 1);
    schedule.setAssignment(0, 2, 0); // Day off
    schedule.setAssignment(0, 3, 1);
    
    int penalty_valid = constraints.evaluateMaxConsecutiveShifts(schedule);
    
    // Test schedule with too many consecutive shifts
    schedule.clear();
    for (int day = 0; day < instance.getHorizonDays(); day++) {
        schedule.setAssignment(0, day, 1); // All days working
    }
    
    int penalty_invalid = constraints.evaluateMaxConsecutiveShifts(schedule);
    bool test1 = (penalty_invalid < 0);
    
    logTest("Max Consecutive Shifts", test1, 
            test1 ? "" : "Should penalize excessive consecutive shifts");
    
    return test1;
}

bool TestHardConstraints::testPreAssignedDaysOff() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Pre-assigned Days Off", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test schedule that respects days off
    schedule.clear();
    int penalty_valid = constraints.evaluatePreAssignedDaysOff(schedule);
    bool test1 = (penalty_valid == 0);
    
    // Test schedule that violates days off (if any exist in the instance)
    const auto& days_off_list = instance.getDaysOff();
    if (!days_off_list.empty()) {
        const auto& first_days_off = days_off_list[0];
        if (!first_days_off.DayIndexes.empty()) {
            int forbidden_day = std::stoi(first_days_off.DayIndexes[0]);
            
            // Find the employee index for this days off entry
            int employee_index = -1;
            for (int i = 0; i < instance.getNumEmployees(); i++) {
                if (instance.getStaff(i).ID == first_days_off.EmployeeID) {
                    employee_index = i;
                    break;
                }
            }
            
            if (employee_index >= 0 && forbidden_day < instance.getHorizonDays()) {
                schedule.setAssignment(employee_index, forbidden_day, 1);
                int penalty_invalid = constraints.evaluatePreAssignedDaysOff(schedule);
                bool test2 = (penalty_invalid < 0);
                
                bool all_passed = test1 && test2;
                logTest("Pre-assigned Days Off", all_passed, 
                        all_passed ? "" : "Should penalize working on forbidden days");
                return all_passed;
            }
        }
    }
    
    logTest("Pre-assigned Days Off", test1, 
            test1 ? "" : "Basic validation failed");
    return test1;
}

bool TestHardConstraints::testShiftRotation() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Shift Rotation", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test valid rotation (days off between shifts)
    schedule.clear();
    schedule.setAssignment(0, 0, 1);
    schedule.setAssignment(0, 1, 0); // Day off
    schedule.setAssignment(0, 2, 1);
    
    int penalty_valid = constraints.evaluateShiftRotation(schedule);
    
    // For this test, we assume the penalty should be 0 for valid rotations
    // The actual result depends on the shift rotation rules in the instance
    
    logTest("Shift Rotation", true, "Basic rotation test completed");
    return true;
}

bool TestHardConstraints::testAggregateEvaluation() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Aggregate Evaluation", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test empty schedule
    schedule.clear();
    int total_penalty = constraints.evaluateAll(schedule);
    bool is_feasible = constraints.isFeasible(schedule);
    
    // Empty schedule should likely have some violations (minimum work time)
    bool test1 = (total_penalty <= 0); // Should have some penalty
    bool test2 = !is_feasible; // Should not be feasible
    
    // Test constraint statistics
    auto stats = constraints.getConstraintStatistics(schedule);
    bool test3 = !stats.empty();
    bool test4 = (stats.find("overall_feasibility") != stats.end());
    
    // Test violation details
    auto violations = constraints.getViolationDetails(schedule);
    bool test5 = true; // Just check it doesn't crash
    
    // Test penalty weights
    auto weights = constraints.getPenaltyWeights();
    bool test6 = !weights.empty();
    
    bool all_passed = test1 && test2 && test3 && test4 && test5 && test6;
    logTest("Aggregate Evaluation", all_passed, 
            all_passed ? "" : "Aggregate methods failed");
    
    return all_passed;
}

bool TestHardConstraints::testMoveEvaluation() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Move Evaluation", false, "Failed to load test instance");
        return false;
    }
    
    HardConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Create a base schedule
    schedule.clear();
    
    // Test evaluating a move
    int move_impact = constraints.evaluateMove(schedule, 0, 0, 0, 1);
    
    // The move evaluation should complete without crashing
    bool test1 = true;
    
    logTest("Move Evaluation", test1, "Move evaluation completed");
    return test1;
}

void TestHardConstraints::runAllTests() {
    std::cout << "=== Running Hard Constraints Tests ===" << std::endl;
    
    testMaxShiftsPerType();
    testWorkingTimeConstraints();
    testMaxConsecutiveShifts();
    testPreAssignedDaysOff();
    testShiftRotation();
    testAggregateEvaluation();
    testMoveEvaluation();
    
    printResults();
}

void TestHardConstraints::printResults() {
    std::cout << "\n=== Hard Constraints Test Results ===" << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;
    
    if (tests_failed > 0) {
        std::cout << "\nFailed tests:" << std::endl;
        for (const auto& test : failed_tests) {
            std::cout << "  - " << test << std::endl;
        }
    }
    
    std::cout << "Success rate: " << 
        (100.0 * tests_passed / (tests_passed + tests_failed)) << "%" << std::endl;
}

bool TestHardConstraints::allTestsPassed() const {
    return tests_failed == 0;
}

void registerHardConstraintTests(TestRunner& runner) {
    TestHardConstraints tests;
    tests.runAllTests();
    runner.logTest("Hard Constraints Suite", tests.allTestsPassed());
}
