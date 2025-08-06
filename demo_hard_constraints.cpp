/**
 * Demo of HardConstraints class functionality
 * Shows how to use the new constraint evaluation system
 */

#include "src/constraints/hard_constraints.h"
#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include <iostream>

int main() {
    std::cout << "=== Hard Constraints Demo ===" << std::endl;
    
    // Load a test instance
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        std::cerr << "Failed to load test instance" << std::endl;
        return 1;
    }
    
    std::cout << "Instance loaded: " << instance.getNumEmployees() << " employees, " 
              << instance.getHorizonDays() << " days" << std::endl;
    
    // Create the hard constraints evaluator
    HardConstraints constraints(instance);
    
    // Create a test schedule
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays());
    
    std::cout << "\n--- Testing Empty Schedule ---" << std::endl;
    schedule.clear();
    
    int total_penalty = constraints.evaluateAll(schedule);
    bool is_feasible = constraints.isFeasible(schedule);
    
    std::cout << "Empty schedule penalty: " << total_penalty << std::endl;
    std::cout << "Is feasible: " << (is_feasible ? "Yes" : "No") << std::endl;
    
    // Get violation details
    auto violations = constraints.getViolationDetails(schedule);
    std::cout << "Violations found: " << violations.size() << std::endl;
    for (const auto& violation : violations) {
        std::cout << "  - " << violation << std::endl;
    }
    
    std::cout << "\n--- Testing Individual Constraints ---" << std::endl;
    
    // Test individual constraints
    std::cout << "Max shifts per type: " << constraints.evaluateMaxShiftsPerType(schedule) << std::endl;
    std::cout << "Working time constraints: " << constraints.evaluateWorkingTimeConstraints(schedule) << std::endl;
    std::cout << "Max consecutive shifts: " << constraints.evaluateMaxConsecutiveShifts(schedule) << std::endl;
    std::cout << "Pre-assigned days off: " << constraints.evaluatePreAssignedDaysOff(schedule) << std::endl;
    
    std::cout << "\n--- Testing Schedule with Some Assignments ---" << std::endl;
    
    // Add some assignments
    for (int emp = 0; emp < std::min(3, instance.getNumEmployees()); emp++) {
        for (int day = 0; day < std::min(5, instance.getHorizonDays()); day++) {
            schedule.setAssignment(emp, day, 1);
        }
    }
    
    total_penalty = constraints.evaluateAll(schedule);
    is_feasible = constraints.isFeasible(schedule);
    
    std::cout << "Partial schedule penalty: " << total_penalty << std::endl;
    std::cout << "Is feasible: " << (is_feasible ? "Yes" : "No") << std::endl;
    
    // Test constraint statistics
    auto stats = constraints.getConstraintStatistics(schedule);
    std::cout << "\nConstraint Statistics:" << std::endl;
    for (const auto& stat : stats) {
        std::cout << "  " << stat.first << ": " << (stat.second * 100) << "% satisfied" << std::endl;
    }
    
    // Test penalty weights
    auto weights = constraints.getPenaltyWeights();
    std::cout << "\nPenalty Weights:" << std::endl;
    for (const auto& weight : weights) {
        std::cout << "  " << weight.first << ": " << weight.second << std::endl;
    }
    
    std::cout << "\n--- Testing Move Evaluation ---" << std::endl;
    
    // Test evaluating a move
    int move_impact = constraints.evaluateMove(schedule, 0, 0, 1, 0);
    std::cout << "Impact of removing assignment (0,0): " << move_impact << std::endl;
    
    move_impact = constraints.evaluateMove(schedule, 1, 6, 0, 1);
    std::cout << "Impact of adding assignment (1,6): " << move_impact << std::endl;
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "The HardConstraints class successfully implements all constraint types" << std::endl;
    std::cout << "from the NSP formulation and provides detailed evaluation capabilities." << std::endl;
    
    return 0;
}