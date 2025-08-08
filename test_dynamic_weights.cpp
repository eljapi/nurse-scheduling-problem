#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include "src/constraints/constraint_evaluator.h"
#include "src/metaheuristics/simulated_annealing.h"
#include <iostream>
#include <cassert>

void testDynamicWeights() {
    std::cout << "Testing Dynamic Penalty Weights System..." << std::endl;
    
    // Create a simple test instance
    Instance instance;
    instance.loadFromFile("nsp_instancias/instances1_24/Instance2.txt");
    
    ConstraintEvaluator evaluator(instance);
    
    // Create a schedule with some violations
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    schedule.randomize(instance.getNumShiftTypes());
    
    // Test initial weights (should all be 1.0)
    auto initial_weights = evaluator.getDynamicWeights();
    std::cout << "Initial weights:" << std::endl;
    for (const auto& pair : initial_weights) {
        std::cout << "  Constraint " << static_cast<int>(pair.first) << ": " << pair.second << std::endl;
        assert(pair.second == 1.0); // All weights should start at 1.0
    }
    
    // Update weights based on current violations
    std::cout << "\nUpdating weights based on violations..." << std::endl;
    evaluator.updateDynamicWeights(schedule);
    
    auto updated_weights = evaluator.getDynamicWeights();
    auto violation_counts = evaluator.getViolationCounts();
    
    std::cout << "Updated weights and violation counts:" << std::endl;
    for (const auto& pair : updated_weights) {
        HardConstraintType constraint_type = pair.first;
        double weight = pair.second;
        int violations = violation_counts.at(constraint_type);
        
        std::cout << "  Constraint " << static_cast<int>(constraint_type) 
                  << ": weight=" << weight << ", violations=" << violations << std::endl;
        
        // Weights should have changed from initial values if there were violations
        if (violations > 0) {
            assert(weight > 1.0); // Weight should increase for violated constraints
        }
    }
    
    // Test weight reset
    std::cout << "\nTesting weight reset..." << std::endl;
    evaluator.resetDynamicWeights();
    
    auto reset_weights = evaluator.getDynamicWeights();
    auto reset_violations = evaluator.getViolationCounts();
    
    for (const auto& pair : reset_weights) {
        assert(pair.second == 1.0); // All weights should be back to 1.0
    }
    
    for (const auto& pair : reset_violations) {
        assert(pair.second == 0); // All violation counts should be 0
    }
    
    std::cout << "All weights reset to 1.0 successfully." << std::endl;
    
    // Test weighted evaluation
    std::cout << "\nTesting weighted evaluation..." << std::endl;
    double regular_hard_score = evaluator.getHardConstraintViolations(schedule);
    double weighted_hard_score = evaluator.getWeightedHardConstraintViolations(schedule);
    
    std::cout << "Regular hard constraint score: " << regular_hard_score << std::endl;
    std::cout << "Weighted hard constraint score: " << weighted_hard_score << std::endl;
    
    // With all weights at 1.0, scores should be equal
    assert(std::abs(regular_hard_score - weighted_hard_score) < 1e-6);
    
    std::cout << "Dynamic weights system test passed!" << std::endl;
}

void testDynamicWeightUpdates() {
    std::cout << "\nTesting Dynamic Weight Updates with Violations..." << std::endl;
    
    Instance instance;
    instance.loadFromFile("nsp_instancias/instances1_24/Instance2.txt");
    
    ConstraintEvaluator evaluator(instance);
    
    // Create a schedule that will likely have working time violations
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Create a very aggressive schedule to force violations
    for (int emp = 0; emp < std::min(3, instance.getNumEmployees()); ++emp) {
        for (int day = 0; day < instance.getHorizonDays(); ++day) {
            // Assign different shift types to create various violations
            int shift_type = (day % instance.getNumShiftTypes()) + 1; // Cycle through shift types 1, 2, 3...
            schedule.setAssignment(emp, day, shift_type);
        }
    }
    
    // Also try to create some specific violations
    if (instance.getNumEmployees() > 3) {
        // Employee 3: work only shift type 1 for many days (may violate max shifts per type)
        for (int day = 0; day < std::min(20, instance.getHorizonDays()); ++day) {
            schedule.setAssignment(3, day, 1);
        }
    }
    
    std::cout << "Created schedule with potential working time violations..." << std::endl;
    
    // Check initial constraint violations
    double initial_hard_score = evaluator.getHardConstraintViolations(schedule);
    std::cout << "Initial hard constraint score: " << initial_hard_score << std::endl;
    
    // Debug: Check individual constraint evaluations
    std::cout << "Individual constraint evaluations:" << std::endl;
    std::cout << "  MaxOneShiftPerDay: " << evaluator.hard_constraints.evaluateMaxOneShiftPerDay(schedule) << std::endl;
    std::cout << "  ShiftRotation: " << evaluator.hard_constraints.evaluateShiftRotation(schedule) << std::endl;
    std::cout << "  MaxShiftsPerType: " << evaluator.hard_constraints.evaluateMaxShiftsPerType(schedule) << std::endl;
    std::cout << "  WorkingTimeConstraints: " << evaluator.hard_constraints.evaluateWorkingTimeConstraints(schedule) << std::endl;
    std::cout << "  MaxConsecutiveShifts: " << evaluator.hard_constraints.evaluateMaxConsecutiveShifts(schedule) << std::endl;
    std::cout << "  MinConsecutiveShifts: " << evaluator.hard_constraints.evaluateMinConsecutiveShifts(schedule) << std::endl;
    std::cout << "  MinConsecutiveDaysOff: " << evaluator.hard_constraints.evaluateMinConsecutiveDaysOff(schedule) << std::endl;
    std::cout << "  MaxWeekendsWorked: " << evaluator.hard_constraints.evaluateMaxWeekendsWorked(schedule) << std::endl;
    std::cout << "  PreAssignedDaysOff: " << evaluator.hard_constraints.evaluatePreAssignedDaysOff(schedule) << std::endl;
    
    // Update weights multiple times to simulate SA behavior
    std::cout << "\nSimulating multiple weight updates..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        evaluator.updateDynamicWeights(schedule);
        
        auto weights = evaluator.getDynamicWeights();
        auto violations = evaluator.getViolationCounts();
        
        std::cout << "Update " << (i+1) << " - Weights and violations:" << std::endl;
        for (const auto& pair : weights) {
            HardConstraintType constraint_type = pair.first;
            double weight = pair.second;
            int total_violations = violations.at(constraint_type);
            
            std::cout << "  Constraint " << static_cast<int>(constraint_type) 
                      << ": weight=" << weight << ", total_violations=" << total_violations << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test weighted evaluation
    double regular_score = evaluator.getHardConstraintViolations(schedule);
    double weighted_score = evaluator.getWeightedHardConstraintViolations(schedule);
    
    std::cout << "Final evaluation comparison:" << std::endl;
    std::cout << "  Regular hard score: " << regular_score << std::endl;
    std::cout << "  Weighted hard score: " << weighted_score << std::endl;
    std::cout << "  Difference: " << (weighted_score - regular_score) << std::endl;
    
    std::cout << "Dynamic weight updates test completed!" << std::endl;
}

int main() {
    try {
        testDynamicWeights();
        testDynamicWeightUpdates();
        std::cout << "\nAll dynamic weights tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}