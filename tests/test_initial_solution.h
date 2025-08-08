#ifndef TEST_INITIAL_SOLUTION_H
#define TEST_INITIAL_SOLUTION_H

#include <vector>
#include <string>

/**
 * Test suite for InitialSolutionGenerator class
 */
class TestInitialSolution {
private:
    int tests_passed;
    int tests_failed;
    std::vector<std::string> failed_tests;
    
    void logTest(const std::string& test_name, bool passed, const std::string& message = "");
    
public:
    TestInitialSolution();
    
    bool runAllTests();
    
    // Results
    void printResults();
    bool allTestsPassed() const;
    
private:
    bool testBasicGeneration();
    bool testAnnualLeaveAssignment();
    bool testSolutionQuality();
    bool testMultipleInstances();
};

#endif // TEST_INITIAL_SOLUTION_H