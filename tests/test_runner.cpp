#include "test_runner.h"
#include "test_hard_constraints.h"
#include "test_soft_constraints.h"
#include "../src/core/instance_parser.h"
#include "../src/core/instance.h"
#include "../src/core/data_structures.h"
#include <iostream>
#include <fstream>
#include <chrono>

TestRunner::TestRunner() : tests_passed(0), tests_failed(0) {}

void TestRunner::logTest(const std::string& test_name, bool passed, const std::string& message) {
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

bool TestRunner::runParsingTest(const std::string& instance_file) {
    InstanceParser parser;
    int horizon;
    std::vector<Staff> workers;
    std::vector<Shift> shifts;
    std::vector<DaysOff> days_off;
    std::vector<ShiftOnRequest> shift_on_requests;
    std::vector<ShiftOffRequest> shift_off_requests;
    std::vector<Cover> cover_requirements;
    
    bool success = parser.parseInstance(instance_file, horizon, workers, shifts, 
                                       days_off, shift_on_requests, 
                                       shift_off_requests, cover_requirements);
    
    if (!success) {
        logTest("Parse " + instance_file, false, "Failed to parse file");
        return false;
    }
    
    // Basic validation
    bool valid = true;
    std::string error_msg;
    
    if (horizon <= 0) {
        valid = false;
        error_msg = "Invalid horizon: " + std::to_string(horizon);
    } else if (workers.empty()) {
        valid = false;
        error_msg = "No workers found";
    } else if (shifts.empty()) {
        valid = false;
        error_msg = "No shifts found";
    }
    
    logTest("Parse " + instance_file, valid, error_msg);
    return valid;
}

bool TestRunner::runScheduleTest() {
    // Test Schedule class basic functionality
    Schedule schedule(3, 7, 3);  // 3 employees, 7 days
    
    // Test assignment and retrieval
    schedule.setAssignment(0, 0, 1);
    schedule.setAssignment(1, 3, 2);
    schedule.setAssignment(2, 6, 1);
    
    bool test1 = (schedule.getAssignment(0, 0) == 1);
    bool test2 = (schedule.getAssignment(1, 3) == 2);
    bool test3 = (schedule.getAssignment(2, 6) == 1);
    bool test4 = (schedule.getAssignment(0, 1) == 0);  // Should be 0 (unassigned)
    
    // Test copy functionality
    Schedule copy_schedule = schedule;
    bool test5 = (copy_schedule.getAssignment(0, 0) == 1);
    bool test6 = (copy_schedule.getAssignment(1, 3) == 2);
    
    bool all_passed = test1 && test2 && test3 && test4 && test5 && test6;
    
    logTest("Schedule Basic Operations", all_passed, 
            all_passed ? "" : "Assignment/retrieval/copy failed");
    
    return all_passed;
}

bool TestRunner::runAdvancedScheduleTests() {
    bool all_tests_passed = true;
    
    // Test 1: Constructor and basic properties
    Schedule schedule(5, 7, 3);
    bool test1 = (schedule.getNumEmployees() == 5 && schedule.getHorizonDays() == 7);
    logTest("Schedule Constructor", test1, test1 ? "" : "Wrong dimensions");
    all_tests_passed &= test1;
    
    // Test 2: Assignment operations with bounds checking
    schedule.setAssignment(0, 0, 1);
    schedule.setAssignment(4, 6, 3);  // Last employee, last day
    schedule.setAssignment(-1, 0, 1); // Invalid employee (should be ignored)
    schedule.setAssignment(0, -1, 1); // Invalid day (should be ignored)
    schedule.setAssignment(5, 0, 1);  // Out of bounds employee (should be ignored)
    schedule.setAssignment(0, 7, 1);  // Out of bounds day (should be ignored)
    
    bool test2 = (schedule.getAssignment(0, 0) == 1 && 
                  schedule.getAssignment(4, 6) == 3 &&
                  schedule.getAssignment(-1, 0) == 0 &&  // Should return 0 for invalid indices
                  schedule.getAssignment(0, -1) == 0);
    logTest("Schedule Bounds Checking", test2, test2 ? "" : "Bounds checking failed");
    all_tests_passed &= test2;
    
    // Test 3: Copy constructor and assignment operator
    Schedule copy_constructed(schedule);
    Schedule copy_assigned(2, 3, 3);
    copy_assigned = schedule;
    
    bool test3 = (copy_constructed == schedule && copy_assigned == schedule);
    logTest("Schedule Copy Operations", test3, test3 ? "" : "Copy operations failed");
    all_tests_passed &= test3;
    
    // Test 4: Randomization
    Schedule random_schedule(3, 5, 3);
    random_schedule.randomize(3);
    
    // Check that randomization produced some non-zero values
    bool has_assignments = false;
    for (int i = 0; i < 3 && !has_assignments; i++) {
        for (int j = 0; j < 5 && !has_assignments; j++) {
            if (random_schedule.getAssignment(i, j) != 0) {
                has_assignments = true;
            }
        }
    }
    logTest("Schedule Randomization", has_assignments, 
            has_assignments ? "" : "Randomization produced all zeros");
    all_tests_passed &= has_assignments;
    
    // Test 5: Swap operations
    Schedule swap_schedule(3, 3, 3);
    swap_schedule.setAssignment(0, 0, 1);
    swap_schedule.setAssignment(1, 1, 2);
    swap_schedule.swapAssignments(0, 0, 1, 1);
    
    bool test5 = (swap_schedule.getAssignment(0, 0) == 2 && 
                  swap_schedule.getAssignment(1, 1) == 1);
    logTest("Schedule Swap Operations", test5, test5 ? "" : "Swap failed");
    all_tests_passed &= test5;
    
    // Test 6: Shift count analysis
    Schedule analysis_schedule(2, 5, 3);
    analysis_schedule.setAssignment(0, 0, 1);
    analysis_schedule.setAssignment(0, 1, 1);
    analysis_schedule.setAssignment(0, 2, 2);
    analysis_schedule.setAssignment(0, 3, 1);
    
    int shift1_count = analysis_schedule.getShiftCount(0, 1);
    int shift2_count = analysis_schedule.getShiftCount(0, 2);
    
    bool test6 = (shift1_count == 3 && shift2_count == 1);
    logTest("Schedule Shift Count Analysis", test6, 
            test6 ? "" : "Expected shift1=3, shift2=1, got " + 
                         std::to_string(shift1_count) + "," + std::to_string(shift2_count));
    all_tests_passed &= test6;
    
    // Test 7: Consecutive shifts analysis
    Schedule consecutive_schedule(2, 6, 3);
    consecutive_schedule.setAssignment(0, 0, 1);
    consecutive_schedule.setAssignment(0, 1, 2);
    consecutive_schedule.setAssignment(0, 2, 1);
    // Days 3, 4, 5 are off (0)
    
    int consecutive_shifts = consecutive_schedule.getConsecutiveShifts(0, 0);
    int consecutive_days_off = consecutive_schedule.getConsecutiveDaysOff(0, 3);
    
    bool test7 = (consecutive_shifts == 3 && consecutive_days_off == 3);
    logTest("Schedule Consecutive Analysis", test7, 
            test7 ? "" : "Expected consecutive_shifts=3, days_off=3");
    all_tests_passed &= test7;
    
    // Test 8: Coverage analysis
    Schedule coverage_schedule(4, 3, 3);
    coverage_schedule.setAssignment(0, 0, 1);  // Day 0: 2 employees work shift 1
    coverage_schedule.setAssignment(1, 0, 1);
    coverage_schedule.setAssignment(2, 0, 2);  // Day 0: 1 employee works shift 2
    coverage_schedule.setAssignment(3, 0, 1);  // Day 0: 3 employees work shift 1 total
    
    int coverage_shift1 = coverage_schedule.getCoverage(0, 1);
    int coverage_shift2 = coverage_schedule.getCoverage(0, 2);
    
    bool test8 = (coverage_shift1 == 3 && coverage_shift2 == 1);
    logTest("Schedule Coverage Analysis", test8, 
            test8 ? "" : "Coverage analysis failed");
    all_tests_passed &= test8;
    
    // Test 9: Utilization rate
    Schedule util_schedule(2, 4, 3);
    util_schedule.setAssignment(0, 0, 1);
    util_schedule.setAssignment(0, 1, 1);
    util_schedule.setAssignment(1, 0, 2);
    // 3 working assignments out of 8 total = 37.5%
    
    double utilization = util_schedule.getUtilizationRate();
    bool test9 = (utilization >= 0.37 && utilization <= 0.38);
    logTest("Schedule Utilization Rate", test9, 
            test9 ? "" : "Expected ~0.375, got " + std::to_string(utilization));
    all_tests_passed &= test9;
    
    // Test 10: Raw matrix compatibility
    Schedule matrix_schedule(2, 3, 3);
    matrix_schedule.setAssignment(0, 0, 1);
    matrix_schedule.setAssignment(1, 2, 2);
    
    int** raw_matrix = matrix_schedule.getRawMatrix();
    bool test10 = (raw_matrix[0][0] == 1 && raw_matrix[1][2] == 2);
    
    // Clean up raw matrix
    for (int i = 0; i < 2; i++) {
        delete[] raw_matrix[i];
    }
    delete[] raw_matrix;
    
    logTest("Schedule Raw Matrix Compatibility", test10, 
            test10 ? "" : "Raw matrix conversion failed");
    all_tests_passed &= test10;
    
    // Test 11: Serialization
    Schedule serial_schedule(2, 3, 3);
    serial_schedule.setAssignment(0, 0, 1);
    serial_schedule.setAssignment(0, 1, 2);
    serial_schedule.setAssignment(1, 2, 3);
    
    std::string compact_str = serial_schedule.toCompactString();
    
    Schedule deserialized_schedule(2, 3, 3);
    deserialized_schedule.fromString(compact_str);
    
    bool test11 = (deserialized_schedule == serial_schedule);
    logTest("Schedule Serialization", test11, 
            test11 ? "" : "Serialization/deserialization failed");
    all_tests_passed &= test11;
    
    // Test 12: Memory footprint
    size_t memory_footprint = schedule.getMemoryFootprint();
    bool test12 = (memory_footprint > 0);
    logTest("Schedule Memory Footprint", test12, 
            test12 ? "Memory footprint: " + std::to_string(memory_footprint) + " bytes" : 
                     "Memory footprint calculation failed");
    all_tests_passed &= test12;
    
    // Test 13: Clear operation
    Schedule clear_schedule(2, 3, 3);
    clear_schedule.setAssignment(0, 0, 1);
    clear_schedule.setAssignment(1, 1, 2);
    clear_schedule.clear();
    
    bool is_empty = true;
    for (int i = 0; i < 2 && is_empty; i++) {
        for (int j = 0; j < 3 && is_empty; j++) {
            if (clear_schedule.getAssignment(i, j) != 0) {
                is_empty = false;
            }
        }
    }
    
    bool test13 = is_empty;
    logTest("Schedule Clear Operation", test13, 
            test13 ? "" : "Clear operation failed");
    all_tests_passed &= test13;
    
    return all_tests_passed;
}

bool TestRunner::runInstanceTest(const std::string& instance_file) {
    // Test the new Instance class
    Instance instance;
    
    bool success = instance.loadFromFile(instance_file);
    if (!success) {
        logTest("Instance Load " + instance_file, false, "Failed to load instance");
        return false;
    }
    
    // Basic validation
    bool valid = true;
    std::string error_msg;
    
    if (instance.getHorizonDays() <= 0) {
        valid = false;
        error_msg = "Invalid horizon: " + std::to_string(instance.getHorizonDays());
    } else if (instance.getNumEmployees() <= 0) {
        valid = false;
        error_msg = "No employees found";
    } else if (instance.getNumShiftTypes() <= 0) {
        valid = false;
        error_msg = "No shift types found";
    }
    
    // Test optimized access methods
    if (valid) {
        try {
            // Test staff access by index and ID
            if (instance.getNumEmployees() > 0) {
                const Staff& first_staff = instance.getStaff(0);
                const Staff& same_staff = instance.getStaffById(first_staff.ID);
                if (first_staff.ID != same_staff.ID) {
                    valid = false;
                    error_msg = "Staff lookup inconsistency";
                }
            }
            
            // Test shift access by index and ID
            if (instance.getNumShiftTypes() > 0) {
                const Shift& first_shift = instance.getShift(0);
                const Shift& same_shift = instance.getShiftById(first_shift.ShiftID);
                if (first_shift.ShiftID != same_shift.ShiftID) {
                    valid = false;
                    error_msg = "Shift lookup inconsistency";
                }
            }
            
            // Test validation methods
            bool valid_staff = instance.isValidStaffIndex(0);
            bool invalid_staff = instance.isValidStaffIndex(-1);
            if (!valid_staff || invalid_staff) {
                valid = false;
                error_msg = "Staff index validation failed";
            }
            
        } catch (const std::exception& e) {
            valid = false;
            error_msg = "Exception in optimized access: " + std::string(e.what());
        }
    }
    
    logTest("Instance Load " + instance_file, valid, error_msg);
    return valid;
}

void TestRunner::runBasicTests() {
    std::cout << "=== Running Basic Tests ===" << std::endl;
    
    // Test data structures
    runScheduleTest();
    runAdvancedScheduleTests();
    
    // Test parsing with Instance1
    runParsingTest("nsp_instancias/instances1_24/Instance1.txt");
    
    // Test new Instance class
    runInstanceTest("nsp_instancias/instances1_24/Instance1.txt");
    
    // Test hard constraints
    runHardConstraintsTests();
    
    // Test soft constraints
    runSoftConstraintsTests();
    
    printResults();
}

void TestRunner::runAllTests() {
    std::cout << "=== Running All Tests ===" << std::endl;
    
    runBasicTests();
    
    // Test all available instances
    std::vector<std::string> instances = {
        "Instance1.txt", "Instance2.txt", "Instance3.txt", 
        "Instance6.txt", "Instance9.txt", "Instance10.txt",
        "Instance12.txt", "Instance14.txt", "Instance17.txt", "Instance18.txt"
    };
    
    for (const auto& instance : instances) {
        std::string full_path = "nsp_instancias/instances1_24/" + instance;
        runInstanceTest(full_path);
    }
    
    printResults();
}

void TestRunner::printResults() {
    std::cout << "\n=== Test Results ===" << std::endl;
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

bool TestRunner::allTestsPassed() const {
    return tests_failed == 0;
}

bool TestRunner::compareSchedules(const std::string& schedule1, const std::string& schedule2) {
    // Simple string comparison for now - can be enhanced later
    return schedule1 == schedule2;
}

bool TestRunner::runHardConstraintsTests() {
    std::cout << "\n=== Testing Hard Constraints ===" << std::endl;
    
    TestHardConstraints hard_constraints_test;
    hard_constraints_test.runAllTests();
    
    bool all_passed = hard_constraints_test.allTestsPassed();
    logTest("Hard Constraints Suite", all_passed, 
            all_passed ? "All constraint tests passed" : "Some constraint tests failed");
    
    return all_passed;
}

bool TestRunner::runSoftConstraintsTests() {
    std::cout << "\n=== Testing Soft Constraints ===" << std::endl;
    
    TestSoftConstraints soft_constraints_test;
    soft_constraints_test.runAllTests();
    
    bool all_passed = soft_constraints_test.allTestsPassed();
    logTest("Soft Constraints Suite", all_passed, 
            all_passed ? "All soft constraint tests passed" : "Some soft constraint tests failed");
    
    return all_passed;
}

bool TestRunner::isValidSolution(const std::string& instance_file, const std::string& solution) {
    // Placeholder - will implement when constraint evaluation is ready
    return true;
}
