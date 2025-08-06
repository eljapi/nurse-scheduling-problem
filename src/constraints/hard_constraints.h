#ifndef HARD_CONSTRAINTS_H
#define HARD_CONSTRAINTS_H

#include "../core/data_structures.h"
#include "../core/instance.h"
#include <vector>
#include <string>
#include <map>

/**
 * HardConstraints class implements all hard constraints from the NSP formulation
 * Based on constraints (1-11) from the research paper:
 * 
 * (1) Maximum 1 shift per day
 * (2) Rotation (minimum rest between shifts)
 * (3) Maximum shifts of type t for employee i
 * (4) Min/max minutes worked
 * (5) Maximum consecutive shifts
 * (6) Minimum consecutive shifts
 * (7) Minimum consecutive days off
 * (8) Count weekends worked
 * (9) Maximum weekends worked
 * (10) Pre-assigned days off
 * (11) Coverage requirements (handled separately in soft constraints)
 */
class HardConstraints {
private:
    const Instance& instance;
    
    // Helper methods for constraint evaluation
    bool isValidShiftTransition(int current_shift, int next_shift) const;
    int countConsecutiveWork(const Schedule& schedule, int employee, int start_day) const;
    int countConsecutiveDaysOff(const Schedule& schedule, int employee, int start_day) const;
    int countWeekendsWorked(const Schedule& schedule, int employee) const;
    
public:
    explicit HardConstraints(const Instance& inst);
    
    // Individual constraint evaluation methods
    // Returns penalty score (0 = satisfied, negative = violated)
    
    /**
     * Constraint (1): Maximum 1 shift per day
     * Ensures each employee works at most one shift per day
     */
    int evaluateMaxOneShiftPerDay(const Schedule& schedule) const;
    
    /**
     * Constraint (2): Rotation - minimum rest between shifts
     * Ensures proper rest between incompatible shift types
     */
    int evaluateShiftRotation(const Schedule& schedule) const;
    int evaluateShiftRotation(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (3): Maximum shifts of type t for employee i
     * Limits the number of each shift type per employee
     */
    int evaluateMaxShiftsPerType(const Schedule& schedule) const;
    int evaluateMaxShiftsPerType(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (4): Min/max minutes worked
     * Ensures employees work within their time limits
     */
    int evaluateWorkingTimeConstraints(const Schedule& schedule) const;
    int evaluateWorkingTimeConstraints(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (5): Maximum consecutive shifts
     * Limits consecutive working days
     */
    int evaluateMaxConsecutiveShifts(const Schedule& schedule) const;
    int evaluateMaxConsecutiveShifts(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (6): Minimum consecutive shifts
     * Ensures minimum consecutive working periods
     */
    int evaluateMinConsecutiveShifts(const Schedule& schedule) const;
    int evaluateMinConsecutiveShifts(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (7): Minimum consecutive days off
     * Ensures adequate rest periods
     */
    int evaluateMinConsecutiveDaysOff(const Schedule& schedule) const;
    int evaluateMinConsecutiveDaysOff(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (9): Maximum weekends worked
     * Limits weekend work assignments
     */
    int evaluateMaxWeekendsWorked(const Schedule& schedule) const;
    int evaluateMaxWeekendsWorked(const Schedule& schedule, int employee_id) const;
    
    /**
     * Constraint (10): Pre-assigned days off
     * Enforces mandatory days off
     */
    int evaluatePreAssignedDaysOff(const Schedule& schedule) const;
    int evaluatePreAssignedDaysOff(const Schedule& schedule, int employee_id) const;
    
    // Aggregate evaluation methods
    
    /**
     * Evaluates all hard constraints and returns total penalty
     * @param schedule The schedule to evaluate
     * @return Total penalty score (0 = all constraints satisfied)
     */
    int evaluateAll(const Schedule& schedule) const;
    
    /**
     * Checks if a schedule satisfies all hard constraints
     * @param schedule The schedule to check
     * @return true if feasible, false otherwise
     */
    bool isFeasible(const Schedule& schedule) const;
    
    /**
     * Evaluates constraints for a specific employee
     * @param schedule The schedule to evaluate
     * @param employee Employee index
     * @return Penalty score for this employee
     */
    int evaluateEmployee(const Schedule& schedule, int employee) const;
    
    /**
     * Evaluates the impact of changing a single assignment
     * @param schedule Current schedule
     * @param employee Employee index
     * @param day Day index
     * @param old_shift Current shift assignment
     * @param new_shift Proposed new shift assignment
     * @return Change in penalty score (negative = improvement)
     */
    int evaluateMove(const Schedule& schedule, int employee, int day, 
                     int old_shift, int new_shift) const;
    
    // Detailed constraint information
    
    /**
     * Gets detailed information about constraint violations
     * @param schedule The schedule to analyze
     * @return Vector of constraint violation descriptions
     */
    std::vector<std::string> getViolationDetails(const Schedule& schedule) const;
    
    /**
     * Gets the penalty weights for different constraint types
     * @return Map of constraint names to penalty weights
     */
    std::map<std::string, int> getPenaltyWeights() const;
    
    // Statistics and analysis
    
    /**
     * Gets constraint satisfaction statistics
     * @param schedule The schedule to analyze
     * @return Map of constraint names to satisfaction rates
     */
    std::map<std::string, double> getConstraintStatistics(const Schedule& schedule) const;

    /**
     * Gets a list of assignments that violate hard constraints
     * @param schedule The schedule to analyze
     * @return Vector of pairs (employee, day) of violating assignments
     */
    std::vector<std::pair<int, int>> getViolatingAssignments(const Schedule& schedule) const;
    std::map<std::string, int> getConstraintViolations(const Schedule& schedule) const;
};

#endif // HARD_CONSTRAINTS_H
