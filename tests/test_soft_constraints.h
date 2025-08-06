#ifndef TEST_SOFT_CONSTRAINTS_H
#define TEST_SOFT_CONSTRAINTS_H

#include <string>
#include <vector>

/**
 * Test suite for SoftConstraints class
 */
class TestSoftConstraints {
private:
    int tests_passed;
    int tests_failed;
    std::vector<std::string> failed_tests;
    
    void logTest(const std::string& test_name, bool passed, const std::string& message = "");
    
public:
    TestSoftConstraints();
    
    // Individual constraint tests
    bool testShiftOnRequests();
    bool testShiftOffRequests();
    bool testCoverageRequirements();
    
    // Aggregate functionality tests
    bool testAggregateEvaluation();
    bool testMoveEvaluation();
    bool testEmployeeEvaluation();
    
    // Analysis and reporting tests
    bool testDetailedAnalysis();
    bool testSatisfactionRates();
    bool testRequestAnalysis();
    
    // Test suite runners
    void runAllTests();
    
    // Results
    void printResults();
    bool allTestsPassed() const;
};

#endif // TEST_SOFT_CONSTRAINTS_H