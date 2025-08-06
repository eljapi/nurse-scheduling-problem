#include "test_runner.h"
#include "../src/core/instance_parser.h"
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
    Schedule schedule(3, 7);  // 3 employees, 7 days
    
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

bool TestRunner::runInstanceTest(const std::string& instance_file) {
    // This is a placeholder for now - will be implemented when we have 
    // the full constraint evaluation system
    
    // For now, just test that we can parse the instance
    return runParsingTest(instance_file);
}

void TestRunner::runBasicTests() {
    std::cout << "=== Running Basic Tests ===" << std::endl;
    
    // Test data structures
    runScheduleTest();
    
    // Test parsing with Instance1
    runParsingTest("nsp_instancias/instances1_24/Instance1.txt");
    
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

bool TestRunner::isValidSolution(const std::string& instance_file, const std::string& solution) {
    // Placeholder - will implement when constraint evaluation is ready
    return true;
}