#ifndef CONSTRAINT_EVALUATOR_H
#define CONSTRAINT_EVALUATOR_H

#include "../core/instance.h"
#include "../core/data_structures.h"
#include "hard_constraints.h"
#include "soft_constraints.h"
#include <map>
#include <string>

enum class HardConstraintType {
    MAX_ONE_SHIFT_PER_DAY,
    SHIFT_ROTATION,
    MAX_SHIFTS_PER_TYPE,
    WORKING_TIME_CONSTRAINTS,
    MAX_CONSECUTIVE_SHIFTS,
    MIN_CONSECUTIVE_SHIFTS,
    MIN_CONSECUTIVE_DAYS_OFF,
    MAX_WEEKENDS_WORKED,
    PRE_ASSIGNED_DAYS_OFF
};

class ConstraintEvaluator {
public:
    const Instance& instance;
    HardConstraints hard_constraints;
    SoftConstraints soft_constraints;

private:
    // Dynamic penalty weights for adaptive constraint handling
    std::map<HardConstraintType, double> dynamic_weights;
    std::map<HardConstraintType, int> violation_counts;
    std::map<HardConstraintType, std::string> constraint_names;
    
    void initializeDynamicWeights();
    std::string getConstraintName(HardConstraintType type) const;

public:
    ConstraintEvaluator(const Instance& inst);
    double evaluateSchedule(const Schedule& schedule);
    bool isFeasible(const Schedule& schedule);
    double getHardConstraintViolations(const Schedule& schedule);
    double getSoftConstraintViolations(const Schedule& schedule);
    double getEmployeeHardConstraintViolations(const Schedule& schedule, int employee_id);
    double getEmployeeSoftConstraintViolations(const Schedule& schedule, int employee_id);
    std::vector<std::pair<int, int>> getViolatingAssignments(const Schedule& schedule);
    std::map<std::string, int> getHardConstraintViolationsMap(const Schedule& schedule);
    
    // Dynamic weight management methods
    void updateDynamicWeights(const Schedule& schedule);
    void resetDynamicWeights();
    double getDynamicWeight(HardConstraintType type) const;
    std::map<HardConstraintType, double> getDynamicWeights() const;
    std::map<HardConstraintType, int> getViolationCounts() const;
    
    // Enhanced evaluation with dynamic weights
    double getWeightedHardConstraintViolations(const Schedule& schedule);
};

#endif // CONSTRAINT_EVALUATOR_H
