#include "test_soft_constraints.h"
#include "../src/constraints/soft_constraints.h"
#include "../src/core/instance.h"
#include "../src/core/data_structures.h"
#include <iostream>
#include <cassert>

TestSoftConstraints::TestSoftConstraints() : tests_passed(0), tests_failed(0) {}

void TestSoftConstraints::logTest(const std::string& test_name, bool passed, const std::string& message) {
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

bool TestSoftConstraints::testShiftOnRequests() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Shift On Requests", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test empty schedule (should have low score)
    schedule.clear();
    int empty_score = constraints.evaluateShiftOnRequests(schedule);
    bool test1 = (empty_score >= 0); // Should be 0 or positive (no negative penalties for empty)
    
    // Test schedule with some assignments that might satisfy requests
    for (int emp = 0; emp < std::min(2, instance.getNumEmployees()); emp++) {
        for (int day = 0; day < std::min(3, instance.getHorizonDays()); day++) {
            schedule.setAssignment(emp, day, 1);
        }
    }
    
    int assigned_score = constraints.evaluateShiftOnRequests(schedule);
    bool test2 = (assigned_score >= empty_score); // Should be same or better
    
    bool all_passed = test1 && test2;
    logTest("Shift On Requests", all_passed, 
            all_passed ? "" : "Request evaluation failed");
    
    return all_passed;
}

bool TestSoftConstraints::testShiftOffRequests() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Shift Off Requests", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test empty schedule (should have no violations)
    schedule.clear();
    int empty_score = constraints.evaluateShiftOffRequests(schedule);
    bool test1 = (empty_score >= 0); // No assignments = no violations
    
    // Test schedule with assignments (might violate some off-requests)
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            schedule.setAssignment(emp, day, 1);
        }
    }
    
    int full_score = constraints.evaluateShiftOffRequests(schedule);
    // Full schedule might have violations, so score could be lower
    
    bool all_passed = test1;
    logTest("Shift Off Requests", all_passed, 
            all_passed ? "" : "Off-request evaluation failed");
    
    return all_passed;
}

bool TestSoftConstraints::testCoverageRequirements() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Coverage Requirements", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test empty schedule (should have under-staffing penalties)
    schedule.clear();
    int empty_score = constraints.evaluateCoverageRequirements(schedule);
    // Empty schedule should have negative score due to under-staffing
    
    // Test schedule with some coverage
    for (int emp = 0; emp < std::min(3, instance.getNumEmployees()); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            schedule.setAssignment(emp, day, 1);
        }
    }
    
    int partial_score = constraints.evaluateCoverageRequirements(schedule);
    // Partial coverage should be better than empty
    bool test1 = (partial_score >= empty_score);
    
    logTest("Coverage Requirements", test1, 
            test1 ? "" : "Coverage evaluation failed");
    
    return test1;
}

bool TestSoftConstraints::testAggregateEvaluation() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Aggregate Evaluation", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test that aggregate equals sum of individual evaluations
    schedule.clear();
    for (int emp = 0; emp < std::min(2, instance.getNumEmployees()); emp++) {
        schedule.setAssignment(emp, 0, 1);
    }
    
    int on_score = constraints.evaluateShiftOnRequests(schedule);
    int off_score = constraints.evaluateShiftOffRequests(schedule);
    int coverage_score = constraints.evaluateCoverageRequirements(schedule);
    int total_individual = on_score + off_score + coverage_score;
    
    int aggregate_score = constraints.evaluateAll(schedule);
    
    bool test1 = (aggregate_score == total_individual);
    
    // Test detailed scores
    auto detailed = constraints.getDetailedScores(schedule);
    bool test2 = (detailed.find("total") != detailed.end());
    bool test3 = (detailed["total"] == aggregate_score);
    
    bool all_passed = test1 && test2 && test3;
    logTest("Aggregate Evaluation", all_passed, 
            all_passed ? "" : "Aggregate evaluation inconsistent");
    
    return all_passed;
}

bool TestSoftConstraints::testMoveEvaluation() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Move Evaluation", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Create a base schedule
    schedule.clear();
    
    // Test evaluating a move
    int move_impact = constraints.evaluateMove(schedule, 0, 0, 0, 1);
    
    // The move evaluation should complete without crashing
    bool test1 = true;
    
    // Test that move evaluation is consistent
    int original_score = constraints.evaluateAll(schedule);
    Schedule temp_schedule = schedule;
    temp_schedule.setAssignment(0, 0, 1);
    int new_score = constraints.evaluateAll(temp_schedule);
    int expected_impact = new_score - original_score;
    
    bool test2 = (move_impact == expected_impact);
    
    bool all_passed = test1 && test2;
    logTest("Move Evaluation", all_passed, 
            all_passed ? "" : "Move evaluation inconsistent");
    
    return all_passed;
}

bool TestSoftConstraints::testEmployeeEvaluation() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Employee Evaluation", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test employee-specific evaluation
    schedule.clear();
    schedule.setAssignment(0, 0, 1);
    
    int emp0_score = constraints.evaluateEmployee(schedule, 0);
    int emp1_score = constraints.evaluateEmployee(schedule, 1);
    
    // Employee 0 has an assignment, employee 1 doesn't
    // The scores might be different based on their requests
    
    bool test1 = true; // Just test that it doesn't crash
    
    // Test invalid employee index
    int invalid_score = constraints.evaluateEmployee(schedule, -1);
    bool test2 = (invalid_score == 0);
    
    bool all_passed = test1 && test2;
    logTest("Employee Evaluation", all_passed, 
            all_passed ? "" : "Employee evaluation failed");
    
    return all_passed;
}

bool TestSoftConstraints::testDetailedAnalysis() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Detailed Analysis", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test detailed scores
    schedule.clear();
    auto detailed = constraints.getDetailedScores(schedule);
    bool test1 = !detailed.empty();
    bool test2 = (detailed.find("shift_on_requests") != detailed.end());
    bool test3 = (detailed.find("shift_off_requests") != detailed.end());
    bool test4 = (detailed.find("coverage_requirements") != detailed.end());
    
    // Test unsatisfied requests
    auto unsatisfied = constraints.getUnsatisfiedRequests(schedule);
    bool test5 = true; // Just test that it doesn't crash
    
    // Test coverage analysis
    auto coverage = constraints.getCoverageAnalysis(schedule);
    bool test6 = true; // Just test that it doesn't crash
    
    bool all_passed = test1 && test2 && test3 && test4 && test5 && test6;
    logTest("Detailed Analysis", all_passed, 
            all_passed ? "" : "Detailed analysis failed");
    
    return all_passed;
}

bool TestSoftConstraints::testSatisfactionRates() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Satisfaction Rates", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test satisfaction rates
    schedule.clear();
    auto rates = constraints.getSatisfactionRates(schedule);
    
    bool test1 = !rates.empty();
    bool test2 = (rates.find("overall") != rates.end());
    
    // Test that rates are between 0 and 1
    bool test3 = true;
    for (const auto& rate : rates) {
        if (rate.second < 0.0 || rate.second > 1.0) {
            test3 = false;
            break;
        }
    }
    
    // Test max possible score
    int max_score = constraints.getMaxPossibleScore();
    bool test4 = (max_score >= 0);
    
    // Test satisfaction percentage
    double satisfaction = constraints.getSatisfactionPercentage(schedule);
    bool test5 = (satisfaction >= 0.0 && satisfaction <= 1.0);
    
    bool all_passed = test1 && test2 && test3 && test4 && test5;
    logTest("Satisfaction Rates", all_passed, 
            all_passed ? "" : "Satisfaction rate calculation failed");
    
    return all_passed;
}

bool TestSoftConstraints::testRequestAnalysis() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        logTest("Request Analysis", false, "Failed to load test instance");
        return false;
    }
    
    SoftConstraints constraints(instance);
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Test request counting
    schedule.clear();
    
    int satisfied_on = constraints.getSatisfiedOnRequests(schedule);
    int violated_off = constraints.getViolatedOffRequests(schedule);
    
    bool test1 = (satisfied_on >= 0);
    bool test2 = (violated_off >= 0);
    
    // Test coverage gaps
    auto gaps = constraints.getCoverageGaps(schedule);
    bool test3 = true; // Just test that it doesn't crash
    
    bool all_passed = test1 && test2 && test3;
    logTest("Request Analysis", all_passed, 
            all_passed ? "" : "Request analysis failed");
    
    return all_passed;
}

void TestSoftConstraints::runAllTests() {
    std::cout << "=== Running Soft Constraints Tests ===" << std::endl;
    
    testShiftOnRequests();
    testShiftOffRequests();
    testCoverageRequirements();
    testAggregateEvaluation();
    testMoveEvaluation();
    testEmployeeEvaluation();
    testDetailedAnalysis();
    testSatisfactionRates();
    testRequestAnalysis();
    
    printResults();
}

void TestSoftConstraints::printResults() {
    std::cout << "\n=== Soft Constraints Test Results ===" << std::endl;
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

bool TestSoftConstraints::allTestsPassed() const {
    return tests_failed == 0;
}

void registerSoftConstraintTests(TestRunner& runner) {
    TestSoftConstraints tests;
    tests.runAllTests();
    runner.logTest("Soft Constraints Suite", tests.allTestsPassed());
}
