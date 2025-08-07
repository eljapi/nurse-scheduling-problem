#ifndef INITIAL_SOLUTION_H
#define INITIAL_SOLUTION_H

#include "../core/data_structures.h"
#include "../core/instance.h"
#include <vector>
#include <set>
#include <random>

/**
 * Generates feasible initial solutions using the 5-step heuristic from research
 * 
 * The algorithm follows these steps:
 * 1. Assign annual leave (PreAssignedDaysOff) by blocking those cells
 * 2. Cover weekend requirements ensuring each nurse has ≥2 weekends off
 * 3. Assign shifts for first 4 days considering previous schedule constraints
 * 4. Iterate day-by-day for remaining horizon, assigning shifts to meet coverage
 * 5. Adjust working hours by adding shifts to meet minimum hour requirements
 */
class InitialSolutionGenerator {
private:
    const Instance& instance;
    std::mt19937 rng;
    
    // Helper structures for tracking assignments
    struct EmployeeState {
        std::set<int> blocked_days;           // Days off (annual leave)
        int consecutive_work_days;            // Current consecutive work streak
        int consecutive_days_off;             // Current consecutive days off streak
        int total_minutes_worked;             // Total minutes worked so far
        int weekends_worked;                  // Number of weekends worked
        int last_shift_type;                  // Last shift type assigned (-1 if none)
        
        EmployeeState() : consecutive_work_days(0), consecutive_days_off(0), 
                         total_minutes_worked(0), weekends_worked(0), last_shift_type(-1) {}
    };
    
    // Coverage tracking for each day/shift combination
    struct CoverageState {
        std::vector<std::vector<int>> current_coverage;  // [day][shift_type] = count
        std::vector<std::vector<int>> required_coverage; // [day][shift_type] = requirement
        
        CoverageState(int days, int shift_types) 
            : current_coverage(days, std::vector<int>(shift_types, 0)),
              required_coverage(days, std::vector<int>(shift_types, 0)) {}
    };
    
public:
    InitialSolutionGenerator(const Instance& instance);
    
    /**
     * Generates a feasible initial solution using the 5-step heuristic
     * @return A schedule that satisfies most hard constraints
     */
    Schedule generateFeasibleSolution();
    
private:
    // Step 1: Assign annual leave (PreAssignedDaysOff) by blocking those cells
    void assignAnnualLeave(Schedule& schedule, std::vector<EmployeeState>& employee_states);
    
    // Step 2: Cover weekend requirements ensuring each nurse has ≥2 weekends off
    void assignWeekends(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage);
    
    // Step 3: Assign shifts for first 4 days considering previous schedule constraints
    void assignInitialDays(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage);
    
    // Step 4: Iterate day-by-day for remaining horizon, assigning shifts to meet coverage
    void assignRemainingHorizon(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage);
    
    // Step 5: Adjust working hours by adding shifts to meet minimum hour requirements
    void adjustWorkingHours(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage);
    
    // Helper methods
    bool canAssignShift(int employee, int day, int shift, const Schedule& schedule, 
                       const std::vector<EmployeeState>& employee_states) const;
    
    std::vector<int> getAvailableEmployees(int day, int shift, const Schedule& schedule, 
                                          const std::vector<EmployeeState>& employee_states) const;
    
    std::vector<int> getUnderCoveredShifts(int day, const CoverageState& coverage) const;
    
    std::vector<int> getEmployeesNeedingMoreHours(const std::vector<EmployeeState>& employee_states) const;
    
    bool isWeekend(int day) const;
    
    int getWeekendNumber(int day) const;
    
    void updateEmployeeState(EmployeeState& state, int day, int shift, const Schedule& schedule);
    
    void updateCoverageState(CoverageState& coverage, int day, int shift, int delta);
    
    // Constraint checking helpers
    bool violatesMaxConsecutiveShifts(int employee, int day, int shift, 
                                     const std::vector<EmployeeState>& employee_states) const;
    
    bool violatesShiftSequence(int employee, int day, int shift, 
                              const std::vector<EmployeeState>& employee_states) const;
    
    bool violatesMaxTotalMinutes(int employee, int shift, 
                                const std::vector<EmployeeState>& employee_states) const;
    
    bool violatesMaxWeekends(int employee, int day, 
                            const std::vector<EmployeeState>& employee_states) const;
    
    // Initialization helpers
    void initializeCoverageRequirements(CoverageState& coverage);
    
    void initializeEmployeeStates(std::vector<EmployeeState>& employee_states);
    
    // Debugging and validation
    void validateSolution(const Schedule& schedule) const;
    
    void printGenerationStats(const Schedule& schedule, const std::vector<EmployeeState>& employee_states) const;
};

#endif // INITIAL_SOLUTION_H