/**
 * Demo of SoftConstraints class functionality
 * Shows how to use the new soft constraint evaluation system
 */

#include "src/constraints/soft_constraints.h"
#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Soft Constraints Demo ===" << std::endl;
    
    // Load a test instance
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        std::cerr << "Failed to load test instance" << std::endl;
        return 1;
    }
    
    std::cout << "Instance loaded: " << instance.getNumEmployees() << " employees, " 
              << instance.getHorizonDays() << " days" << std::endl;
    std::cout << "Shift-on requests: " << instance.getShiftOnRequests().size() << std::endl;
    std::cout << "Shift-off requests: " << instance.getShiftOffRequests().size() << std::endl;
    std::cout << "Coverage requirements: " << instance.getCoverageRequirements().size() << std::endl;
    
    // Create the soft constraints evaluator
    SoftConstraints constraints(instance);
    
    // Create test schedules
    Schedule empty_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    Schedule partial_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    Schedule full_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    std::cout << "\n--- Testing Empty Schedule ---" << std::endl;
    empty_schedule.clear();
    
    int empty_total = constraints.evaluateAll(empty_schedule);
    std::cout << "Empty schedule total score: " << empty_total << std::endl;
    
    // Individual constraint scores
    std::cout << "Individual scores:" << std::endl;
    std::cout << "  Shift-on requests: " << constraints.evaluateShiftOnRequests(empty_schedule) << std::endl;
    std::cout << "  Shift-off requests: " << constraints.evaluateShiftOffRequests(empty_schedule) << std::endl;
    std::cout << "  Coverage requirements: " << constraints.evaluateCoverageRequirements(empty_schedule) << std::endl;
    
    std::cout << "\n--- Testing Partial Schedule ---" << std::endl;
    partial_schedule.clear();
    
    // Add some assignments that might satisfy some requests
    for (int emp = 0; emp < std::min(3, instance.getNumEmployees()); emp++) {
        for (int day = 0; day < std::min(5, instance.getHorizonDays()); day++) {
            partial_schedule.setAssignment(emp, day, 1);
        }
    }
    
    int partial_total = constraints.evaluateAll(partial_schedule);
    std::cout << "Partial schedule total score: " << partial_total << std::endl;
    std::cout << "Individual scores:" << std::endl;
    std::cout << "  Shift-on requests: " << constraints.evaluateShiftOnRequests(partial_schedule) << std::endl;
    std::cout << "  Shift-off requests: " << constraints.evaluateShiftOffRequests(partial_schedule) << std::endl;
    std::cout << "  Coverage requirements: " << constraints.evaluateCoverageRequirements(partial_schedule) << std::endl;
    
    std::cout << "\n--- Testing Full Schedule ---" << std::endl;
    full_schedule.clear();
    
    // Assign all employees to work all days
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            full_schedule.setAssignment(emp, day, 1);
        }
    }
    
    int full_total = constraints.evaluateAll(full_schedule);
    std::cout << "Full schedule total score: " << full_total << std::endl;
    std::cout << "Individual scores:" << std::endl;
    std::cout << "  Shift-on requests: " << constraints.evaluateShiftOnRequests(full_schedule) << std::endl;
    std::cout << "  Shift-off requests: " << constraints.evaluateShiftOffRequests(full_schedule) << std::endl;
    std::cout << "  Coverage requirements: " << constraints.evaluateCoverageRequirements(full_schedule) << std::endl;
    
    std::cout << "\n--- Detailed Analysis ---" << std::endl;
    
    // Show detailed scores for partial schedule
    auto detailed = constraints.getDetailedScores(partial_schedule);
    std::cout << "Detailed scores for partial schedule:" << std::endl;
    for (const auto& score : detailed) {
        std::cout << "  " << std::setw(20) << score.first << ": " << score.second << std::endl;
    }
    
    // Show satisfaction rates
    auto rates = constraints.getSatisfactionRates(partial_schedule);
    std::cout << "\nSatisfaction rates for partial schedule:" << std::endl;
    for (const auto& rate : rates) {
        std::cout << "  " << std::setw(20) << rate.first << ": " 
                  << std::setw(6) << std::fixed << std::setprecision(1) 
                  << (rate.second * 100) << "%" << std::endl;
    }
    
    // Show unsatisfied requests
    auto unsatisfied = constraints.getUnsatisfiedRequests(partial_schedule);
    std::cout << "\nUnsatisfied requests (" << unsatisfied.size() << " total):" << std::endl;
    for (size_t i = 0; i < std::min(size_t(5), unsatisfied.size()); i++) {
        std::cout << "  " << unsatisfied[i] << std::endl;
    }
    if (unsatisfied.size() > 5) {
        std::cout << "  ... and " << (unsatisfied.size() - 5) << " more" << std::endl;
    }
    
    // Show request analysis
    std::cout << "\n--- Request Analysis ---" << std::endl;
    std::cout << "Satisfied on-requests: " << constraints.getSatisfiedOnRequests(partial_schedule) << std::endl;
    std::cout << "Violated off-requests: " << constraints.getViolatedOffRequests(partial_schedule) << std::endl;
    
    int max_possible = constraints.getMaxPossibleScore();
    double satisfaction_pct = constraints.getSatisfactionPercentage(partial_schedule);
    std::cout << "Max possible score: " << max_possible << std::endl;
    std::cout << "Satisfaction percentage: " << (satisfaction_pct * 100) << "%" << std::endl;
    
    // Show coverage gaps
    auto gaps = constraints.getCoverageGaps(partial_schedule);
    std::cout << "\nCoverage gaps (first 5):" << std::endl;
    int count = 0;
    for (const auto& gap : gaps) {
        if (count++ >= 5) break;
        std::cout << "  " << gap.first << ": " << gap.second 
                  << (gap.second > 0 ? " (over)" : gap.second < 0 ? " (under)" : " (exact)") << std::endl;
    }
    
    std::cout << "\n--- Move Evaluation ---" << std::endl;
    
    // Test evaluating moves
    int move_impact1 = constraints.evaluateMove(empty_schedule, 0, 0, 0, 1);
    int move_impact2 = constraints.evaluateMove(full_schedule, 0, 0, 1, 0);
    
    std::cout << "Impact of adding work to empty schedule (0,0): " << move_impact1 << std::endl;
    std::cout << "Impact of removing work from full schedule (0,0): " << move_impact2 << std::endl;
    
    // Test employee-specific evaluation
    std::cout << "\n--- Employee-Specific Analysis ---" << std::endl;
    for (int emp = 0; emp < std::min(3, instance.getNumEmployees()); emp++) {
        int emp_score = constraints.evaluateEmployee(partial_schedule, emp);
        std::cout << "Employee " << emp << " soft constraint score: " << emp_score << std::endl;
    }
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "The SoftConstraints class successfully implements all soft constraint types" << std::endl;
    std::cout << "and provides comprehensive analysis capabilities for employee requests" << std::endl;
    std::cout << "and coverage requirements." << std::endl;
    
    return 0;
}
