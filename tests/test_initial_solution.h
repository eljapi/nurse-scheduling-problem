#ifndef TEST_INITIAL_SOLUTION_H
#define TEST_INITIAL_SOLUTION_H

/**
 * Test suite for InitialSolutionGenerator class
 */
class TestInitialSolution {
public:
    TestInitialSolution();
    
    bool runAllTests();
    
private:
    bool testBasicGeneration();
    bool testAnnualLeaveAssignment();
    bool testSolutionQuality();
};

#endif // TEST_INITIAL_SOLUTION_H