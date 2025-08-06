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

double ConstraintEvaluator::getSoftConstraintPenalties(const Schedule& schedule) {
    double score = 0;
    score += soft_constraints.evaluateShiftOnRequests(schedule);
    score += soft_constraints.evaluateShiftOffRequests(schedule);
    score += soft_constraints.evaluateCoverageRequirements(schedule);
    return score;
}

double ConstraintEvaluator::evaluateSchedule(const Schedule& schedule) {
    double hard_violations = getHardConstraintViolations(schedule);
    if (hard_violations < 0) {
        return hard_violations;
    }
    return getSoftConstraintPenalties(schedule);
}

bool ConstraintEvaluator::isFeasible(const Schedule& schedule) {
    return getHardConstraintViolations(schedule) == 0;
}
