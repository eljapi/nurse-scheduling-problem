#ifndef TEST_SOLUTION_VALIDATOR_H
#define TEST_SOLUTION_VALIDATOR_H

#include "test_runner.h"
#include "../src/core/instance.h"
#include "../src/core/data_structures.h"
#include <string>

Schedule parseSolution(const std::string& solution_file, const Instance& instance);
bool testSolutionFeasibility();
void registerSolutionValidatorTests(TestRunner& runner);

#endif // TEST_SOLUTION_VALIDATOR_H
