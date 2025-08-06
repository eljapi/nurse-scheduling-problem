#ifndef TEST_HARD_CONSTRAINTS_H
#define TEST_HARD_CONSTRAINTS_H

#include "test_runner.h"
#include <string>
#include <vector>

/**
 * Test suite for HardConstraints class
 */
class TestHardConstraints {
private:
    int tests_passed;
    int tests_failed;
    std::vector<std::string> failed_tests;
    
    void logTest(const std::string& test_name, bool passed, const std::string& message = "");
    
public:
    TestHardConstraints();
    
    // Individual constraint tests
    bool testMaxShiftsPerType();
    bool testWorkingTimeConstraints();
    bool testMaxConsecutiveShifts();
    bool testPreAssignedDaysOff();
    bool testShiftRotation();
    
    // Aggregate functionality tests
    bool testAggregateEvaluation();
    bool testMoveEvaluation();
    
    // Test suite runners
    void runAllTests();
    
    // Results
    void printResults();
    bool allTestsPassed() const;
};

void registerHardConstraintTests(TestRunner& runner);

#endif // TEST_HARD_CONSTRAINTS_H
