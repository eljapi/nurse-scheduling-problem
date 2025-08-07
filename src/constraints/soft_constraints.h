#ifndef SOFT_CONSTRAINTS_H
#define SOFT_CONSTRAINTS_H

#include "../core/data_structures.h"
#include "../core/instance.h"
#include <vector>
#include <string>
#include <map>

/**
 * SoftConstraints class implements all soft constraints from the NSP formulation
 * These constraints represent preferences and coverage requirements that can be violated
 * with penalties, unlike hard constraints which must be satisfied.
 * 
 * Based on the objective function components:
 * - Employee shift requests (on/off)
 * - Coverage requirements (under/over staffing)
 */
class SoftConstraints {
private:
    const Instance& instance;
    
    // Helper methods
    int findEmployeeIndex(const std::string& employee_id) const;
    int findShiftIndex(const std::string& shift_id) const;
    
public:
    explicit SoftConstraints(const Instance& inst);
    
    // Individual soft constraint evaluation methods
    // Returns penalty/reward score (positive = good, negative = penalty)
    
    /**
     * Evaluates shift-on requests (employees want to work specific shifts)
     * Rewards when employees get their requested shifts
     * @param schedule The schedule to evaluate
     * @return Positive score for satisfied requests
     */
    int evaluateShiftOnRequests(const Schedule& schedule) const;
    int evaluateShiftOnRequests(const Schedule& schedule, int employee_id) const;
    
    /**
     * Evaluates shift-off requests (employees want to avoid specific shifts)
     * Penalizes when employees are assigned shifts they want to avoid
     * @param schedule The schedule to evaluate
     * @return Negative score for violated off-requests
     */
    int evaluateShiftOffRequests(const Schedule& schedule) const;
    int evaluateShiftOffRequests(const Schedule& schedule, int employee_id) const;
    
    /**
     * Evaluates coverage requirements for all shifts and days
     * Penalizes under-staffing and over-staffing based on requirements
     * @param schedule The schedule to evaluate
     * @return Score based on coverage satisfaction (negative for violations)
     */
    int evaluateCoverageRequirements(const Schedule& schedule) const;
    
    // Aggregate evaluation methods
    
    /**
     * Evaluates all soft constraints and returns total score
     * @param schedule The schedule to evaluate
     * @return Total soft constraint score (higher is better)
     */
    int evaluateAll(const Schedule& schedule) const;
    
    /**
     * Evaluates soft constraints for a specific employee
     * @param schedule The schedule to evaluate
     * @param employee Employee index
     * @return Soft constraint score for this employee
     */
    int evaluateEmployee(const Schedule& schedule, int employee) const;
    
    /**
     * Evaluates the impact of changing a single assignment on soft constraints
     * @param schedule Current schedule
     * @param employee Employee index
     * @param day Day index
     * @param old_shift Current shift assignment
     * @param new_shift Proposed new shift assignment
     * @return Change in soft constraint score (positive = improvement)
     */
    int evaluateMove(const Schedule& schedule, int employee, int day, 
                     int old_shift, int new_shift) const;
    
    // Detailed analysis methods
    
    /**
     * Gets detailed breakdown of soft constraint scores
     * @param schedule The schedule to analyze
     * @return Map of constraint names to scores
     */
    std::map<std::string, int> getDetailedScores(const Schedule& schedule) const;
    
    /**
     * Gets information about unsatisfied requests
     * @param schedule The schedule to analyze
     * @return Vector of descriptions of unsatisfied requests
     */
    std::vector<std::string> getUnsatisfiedRequests(const Schedule& schedule) const;
    
    /**
     * Gets coverage analysis for all days and shifts
     * @param schedule The schedule to analyze
     * @return Map with coverage information
     */
    std::map<std::string, std::vector<int>> getCoverageAnalysis(const Schedule& schedule) const;
    
    // Statistics and reporting
    
    /**
     * Gets satisfaction rates for different types of soft constraints
     * @param schedule The schedule to analyze
     * @return Map of constraint types to satisfaction rates (0.0 to 1.0)
     */
    std::map<std::string, double> getSatisfactionRates(const Schedule& schedule) const;
    
    /**
     * Gets the total possible score if all soft constraints were satisfied
     * @return Maximum possible soft constraint score
     */
    int getMaxPossibleScore() const;
    
    /**
     * Gets the satisfaction percentage for the given schedule
     * @param schedule The schedule to analyze
     * @return Satisfaction percentage (0.0 to 1.0)
     */
    double getSatisfactionPercentage(const Schedule& schedule) const;
    
    // Request analysis
    
    /**
     * Gets count of satisfied shift-on requests
     * @param schedule The schedule to analyze
     * @return Number of satisfied on-requests
     */
    int getSatisfiedOnRequests(const Schedule& schedule) const;
    
    /**
     * Gets count of violated shift-off requests
     * @param schedule The schedule to analyze
     * @return Number of violated off-requests
     */
    int getViolatedOffRequests(const Schedule& schedule) const;
    
    /**
     * Gets coverage deficit/surplus for each day and shift
     * @param schedule The schedule to analyze
     * @return Map with coverage gaps
     */
    std::map<std::string, int> getCoverageGaps(const Schedule& schedule) const;
    
    // Incremental evaluation methods
    
    /**
     * Calcula el delta en la puntuación de las preferencias de un empleado (On/Off Requests).
     */
    int calculateEmployeeDelta(const Schedule& schedule, int employee_id, int day, int new_shift) const;
    
    /**
     * Calcula el delta en la puntuación de cobertura para un día específico
     * si un turno se deja y otro se toma.
     */
    int calculateCoverageDelta(const Schedule& schedule, int day, int old_shift, int new_shift) const;
};

#endif // SOFT_CONSTRAINTS_H
