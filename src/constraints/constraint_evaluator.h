#ifndef CONSTRAINT_EVALUATOR_H
#define CONSTRAINT_EVALUATOR_H

#include "hard_constraints.h"
#include "soft_constraints.h"
#include "../core/data_structures.h"
#include "../core/instance.h"
#include <map>
#include <string>
#include <vector>

/**
 * ConstraintEvaluator class provides a unified interface for evaluating
 * both hard and soft constraints in the NSP problem.
 * 
 * This class combines the functionality of HardConstraints and SoftConstraints
 * to provide a single point of evaluation that matches the original main.cpp
 * constraint evaluation pattern.
 */
class ConstraintEvaluator {
private:
    const Instance& instance;
    HardConstraints hard_constraints;
    SoftConstraints soft_constraints;
    
public:
    explicit ConstraintEvaluator(const Instance& inst);
    
    // Main evaluation methods that match original main.cpp pattern
    
    /**
     * Evaluates all hard constraints and returns the penalty score
     * This replaces the scattered hard constraint calls in the main loop
     * @param schedule The schedule to evaluate
     * @return Hard constraint penalty score (0 = all satisfied, negative = violations)
     */
    int evaluateHardConstraints(const Schedule& schedule) const;
    
    /**
     * Evaluates all soft constraints and returns the fitness score
     * This replaces the fitness calculation in the main loop
     * @param schedule The schedule to evaluate
     * @return Soft constraint fitness score (higher is better)
     */
    int evaluateSoftConstraints(const Schedule& schedule) const;
    
    /**
     * Evaluates the complete objective function (hard + soft constraints)
     * @param schedule The schedule to evaluate
     * @return Combined score (hard constraint penalties + soft constraint rewards)
     */
    double evaluateTotal(const Schedule& schedule) const;
    
    /**
     * Checks if a schedule satisfies all hard constraints
     * @param schedule The schedule to check
     * @return true if feasible (no hard constraint violations), false otherwise
     */
    bool isFeasible(const Schedule& schedule) const;
    
    // Move evaluation for optimization algorithms
    
    /**
     * Evaluates the impact of changing a single assignment
     * This is used in the simulated annealing neighborhood exploration
     * @param schedule Current schedule
     * @param employee Employee index
     * @param day Day index
     * @param old_shift Current shift assignment
     * @param new_shift Proposed new shift assignment
     * @return Change in total score (positive = improvement)
     */
    double evaluateMove(const Schedule& schedule, int employee, int day, 
                       int old_shift, int new_shift) const;
    
    /**
     * Evaluates only hard constraint changes for a move
     * @param schedule Current schedule
     * @param employee Employee index
     * @param day Day index
     * @param old_shift Current shift assignment
     * @param new_shift Proposed new shift assignment
     * @return Change in hard constraint penalty
     */
    int evaluateHardConstraintMove(const Schedule& schedule, int employee, int day, 
                                  int old_shift, int new_shift) const;
    
    /**
     * Evaluates only soft constraint changes for a move
     * @param schedule Current schedule
     * @param employee Employee index
     * @param day Day index
     * @param old_shift Current shift assignment
     * @param new_shift Proposed new shift assignment
     * @return Change in soft constraint score
     */
    int evaluateSoftConstraintMove(const Schedule& schedule, int employee, int day, 
                                  int old_shift, int new_shift) const;
    
    // Analysis and reporting methods
    
    /**
     * Gets detailed breakdown of all constraint scores
     * @param schedule The schedule to analyze
     * @return Map with detailed constraint scores
     */
    std::map<std::string, double> getDetailedEvaluation(const Schedule& schedule) const;
    
    /**
     * Gets constraint violation details for debugging
     * @param schedule The schedule to analyze
     * @return Vector of violation descriptions
     */
    std::vector<std::string> getViolationReport(const Schedule& schedule) const;
    
    /**
     * Gets constraint satisfaction statistics
     * @param schedule The schedule to analyze
     * @return Map with satisfaction rates for different constraint types
     */
    std::map<std::string, double> getConstraintStatistics(const Schedule& schedule) const;
    
    // Access to individual constraint evaluators
    
    /**
     * Gets reference to the hard constraints evaluator
     * @return Reference to HardConstraints instance
     */
    const HardConstraints& getHardConstraints() const;
    
    /**
     * Gets reference to the soft constraints evaluator
     * @return Reference to SoftConstraints instance
     */
    const SoftConstraints& getSoftConstraints() const;
    
    // Utility methods for compatibility with original code
    
    /**
     * Evaluates constraints for a specific employee
     * @param schedule The schedule to evaluate
     * @param employee Employee index
     * @return Combined constraint score for this employee
     */
    double evaluateEmployee(const Schedule& schedule, int employee) const;
    
    /**
     * Gets the maximum possible soft constraint score
     * @return Maximum achievable soft constraint score
     */
    int getMaxPossibleSoftScore() const;
    
    /**
     * Gets the satisfaction percentage for soft constraints
     * @param schedule The schedule to analyze
     * @return Satisfaction percentage (0.0 to 1.0)
     */
    double getSoftConstraintSatisfactionRate(const Schedule& schedule) const;
};

#endif // CONSTRAINT_EVALUATOR_H