#include "constraint_evaluator.h"

ConstraintEvaluator::ConstraintEvaluator(const Instance& inst)
    : instance(inst), hard_constraints(inst), soft_constraints(inst) {}

double ConstraintEvaluator::getHardConstraintViolations(const Schedule& schedule) {
    double score = 0;
    score += hard_constraints.evaluateMaxShiftsPerType(schedule);
    score += hard_constraints.evaluateWorkingTimeConstraints(schedule);
    score += hard_constraints.evaluateMaxConsecutiveShifts(schedule);
    score += hard_constraints.evaluateMinConsecutiveShifts(schedule);
    score += hard_constraints.evaluateMaxWeekendsWorked(schedule);
    score += hard_constraints.evaluatePreAssignedDaysOff(schedule);
    score += hard_constraints.evaluateShiftRotation(schedule);
    return score;
}

double ConstraintEvaluator::getSoftConstraintViolations(const Schedule& schedule) {
    double score = 0;
    score += soft_constraints.evaluateShiftOnRequests(schedule);
    score += soft_constraints.evaluateShiftOffRequests(schedule);
    score += soft_constraints.evaluateCoverageRequirements(schedule);
    return score;
}

double ConstraintEvaluator::evaluateSchedule(const Schedule& schedule) {
    double hard_violations = getHardConstraintViolations(schedule);
    if (hard_violations > 0) {
        return hard_violations;
    }
    return getSoftConstraintViolations(schedule);
}

bool ConstraintEvaluator::isFeasible(const Schedule& schedule) {
    return getHardConstraintViolations(schedule) == 0;
}

double ConstraintEvaluator::getEmployeeHardConstraintViolations(const Schedule& schedule, int employee_id) {
    double score = 0;
    score += hard_constraints.evaluateMaxShiftsPerType(schedule, employee_id);
    score += hard_constraints.evaluateWorkingTimeConstraints(schedule, employee_id);
    score += hard_constraints.evaluateMaxConsecutiveShifts(schedule, employee_id);
    score += hard_constraints.evaluateMinConsecutiveShifts(schedule, employee_id);
    score += hard_constraints.evaluateMaxWeekendsWorked(schedule, employee_id);
    score += hard_constraints.evaluatePreAssignedDaysOff(schedule, employee_id);
    score += hard_constraints.evaluateShiftRotation(schedule, employee_id);
    return score;
}

double ConstraintEvaluator::getEmployeeSoftConstraintViolations(const Schedule& schedule, int employee_id) {
    double score = 0;
    score += soft_constraints.evaluateShiftOnRequests(schedule, employee_id);
    score += soft_constraints.evaluateShiftOffRequests(schedule, employee_id);
    // Coverage requirements are not employee-specific
    return score;
}

std::vector<std::pair<int, int>> ConstraintEvaluator::getViolatingAssignments(const Schedule& schedule) {
    return hard_constraints.getViolatingAssignments(schedule);
}

std::map<std::string, int> ConstraintEvaluator::getHardConstraintViolationsMap(const Schedule& schedule) {
    return hard_constraints.getConstraintViolations(schedule);
}
