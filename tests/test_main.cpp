#include "test_runner.h"
#include <iostream>

int main(int argc, char* argv[]) {
    TestRunner runner;
    
    if (argc > 1 && std::string(argv[1]) == "--all") {
        runner.runAllTests();
    } else {
        runner.runBasicTests();
    }
    
    return runner.allTestsPassed() ? 0 : 1;
}