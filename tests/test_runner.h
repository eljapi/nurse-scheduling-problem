#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <string>
#include <vector>

/**
 * Basic test framework for NSP optimization project
 */
class TestRunner {
private:
    int tests_passed;
    int tests_failed;
    std::vector<std::string> failed_tests;
    
public:
    void logTest(const std::string& test_name, bool passed, const std::string& message = "");
    TestRunner();
    
    // Test execution methods
    bool runInstanceTest(const std::string& instance_file);
    bool runParsingTest(const std::string& instance_file);
    bool runScheduleTest();
    bool runAdvancedScheduleTests();
    bool runHardConstraintsTests();
    bool runSoftConstraintsTests();
    
    // Test suite runners
    void runAllTests();
    void runBasicTests();
    
    // Results
    void printResults();
    bool allTestsPassed() const;
    
    // Utility methods
    bool compareSchedules(const std::string& schedule1, const std::string& schedule2);
    bool isValidSolution(const std::string& instance_file, const std::string& solution);
};

#endif // TEST_RUNNER_H
