#include "test_runner.h"
#include "test_hard_constraints.h"
#include "test_soft_constraints.h"
#include "test_solution_validator.h"
#include "test_instance10_validator.h"
// #include "test_solution_validator.cpp"

void registerHardConstraintTests(TestRunner& runner);
void registerSoftConstraintTests(TestRunner& runner);
void registerSolutionValidatorTests(TestRunner& runner);
void registerInstance10ValidatorTests(TestRunner& runner);

int main() {
    TestRunner runner;
    
    registerHardConstraintTests(runner);
    registerSoftConstraintTests(runner);
    registerSolutionValidatorTests(runner);
    registerInstance10ValidatorTests(runner);
    
    runner.runAllTests();
    
    return 0;
}
