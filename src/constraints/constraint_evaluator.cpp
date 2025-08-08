#include "constraint_evaluator.h"
#include <iostream>

ConstraintEvaluator::ConstraintEvaluator(const Instance& inst)
    : instance(inst), hard_constraints(inst), soft_constraints(inst) {
    initializeDynamicWeights();
}

double ConstraintEvaluator::getHardConstraintViolations(const Schedule& schedule) {
    double score = 0;
    score += hard_constraints.evaluateMaxShiftsPerType(schedule);
    score += hard_constraints.evaluateWorkingTimeConstraints(schedule);
    score += hard_constraints.evaluateMaxConsecutiveShifts(schedule);
    score += hard_constraints.evaluateMinConsecutiveShifts(schedule);
    score += hard_constraints.evaluateMinConsecutiveDaysOff(schedule);  // This was missing!
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
    if (hard_violations < 0) {  // Hard constraints violated (negative penalty)
        return hard_violations;
    }
    return getSoftConstraintViolations(schedule);  // Feasible solution, optimize soft constraints
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
    score += hard_constraints.evaluateMinConsecutiveDaysOff(schedule, employee_id);  // This was missing!
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

void ConstraintEvaluator::initializeDynamicWeights() {
    // Initialize constraint names mapping
    constraint_names[HardConstraintType::MAX_ONE_SHIFT_PER_DAY] = "MaxOneShiftPerDay";
    constraint_names[HardConstraintType::SHIFT_ROTATION] = "ShiftRotation";
    constraint_names[HardConstraintType::MAX_SHIFTS_PER_TYPE] = "MaxShiftsPerType";
    constraint_names[HardConstraintType::WORKING_TIME_CONSTRAINTS] = "WorkingTimeConstraints";
    constraint_names[HardConstraintType::MAX_CONSECUTIVE_SHIFTS] = "MaxConsecutiveShifts";
    constraint_names[HardConstraintType::MIN_CONSECUTIVE_SHIFTS] = "MinConsecutiveShifts";
    constraint_names[HardConstraintType::MIN_CONSECUTIVE_DAYS_OFF] = "MinConsecutiveDaysOff";
    constraint_names[HardConstraintType::MAX_WEEKENDS_WORKED] = "MaxWeekendsWorked";
    constraint_names[HardConstraintType::PRE_ASSIGNED_DAYS_OFF] = "PreAssignedDaysOff";
    
    // Initialize all weights to 1.0 (base penalty)
    for (const auto& pair : constraint_names) {
        dynamic_weights[pair.first] = 1.0;
        violation_counts[pair.first] = 0;
    }
}

std::string ConstraintEvaluator::getConstraintName(HardConstraintType type) const {
    auto it = constraint_names.find(type);
    return (it != constraint_names.end()) ? it->second : "Unknown";
}

void ConstraintEvaluator::updateDynamicWeights(const Schedule& schedule) {
    // Count current violations for each constraint type
    std::map<HardConstraintType, int> current_violations;
    
    // Evaluate each constraint type and count violations
    current_violations[HardConstraintType::MAX_ONE_SHIFT_PER_DAY] = 
        (hard_constraints.evaluateMaxOneShiftPerDay(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::SHIFT_ROTATION] = 
        (hard_constraints.evaluateShiftRotation(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::MAX_SHIFTS_PER_TYPE] = 
        (hard_constraints.evaluateMaxShiftsPerType(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::WORKING_TIME_CONSTRAINTS] = 
        (hard_constraints.evaluateWorkingTimeConstraints(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::MAX_CONSECUTIVE_SHIFTS] = 
        (hard_constraints.evaluateMaxConsecutiveShifts(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::MIN_CONSECUTIVE_SHIFTS] = 
        (hard_constraints.evaluateMinConsecutiveShifts(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::MIN_CONSECUTIVE_DAYS_OFF] = 
        (hard_constraints.evaluateMinConsecutiveDaysOff(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::MAX_WEEKENDS_WORKED] = 
        (hard_constraints.evaluateMaxWeekendsWorked(schedule) < 0) ? 1 : 0;
    current_violations[HardConstraintType::PRE_ASSIGNED_DAYS_OFF] = 
        (hard_constraints.evaluatePreAssignedDaysOff(schedule) < 0) ? 1 : 0;
    
    // Update violation counts and adjust weights
    const double WEIGHT_INCREASE_FACTOR = 1.3;  // 30% increase for violated constraints
    const double WEIGHT_DECREASE_FACTOR = 0.95; // 5% decrease for satisfied constraints
    const double MIN_WEIGHT = 0.1;
    const double MAX_WEIGHT = 10.0;
    
    for (const auto& pair : current_violations) {
        HardConstraintType constraint_type = pair.first;
        int violations = pair.second;
        
        // Update violation count
        violation_counts[constraint_type] += violations;
        
        // Adjust weight based on current violation status
        if (violations > 0) {
            // Increase weight for violated constraints
            dynamic_weights[constraint_type] = std::min(
                dynamic_weights[constraint_type] * WEIGHT_INCREASE_FACTOR, 
                MAX_WEIGHT
            );
        } else {
            // Slightly decrease weight for satisfied constraints
            dynamic_weights[constraint_type] = std::max(
                dynamic_weights[constraint_type] * WEIGHT_DECREASE_FACTOR, 
                MIN_WEIGHT
            );
        }
    }
}

void ConstraintEvaluator::resetDynamicWeights() {
    for (auto& pair : dynamic_weights) {
        pair.second = 1.0;
    }
    for (auto& pair : violation_counts) {
        pair.second = 0;
    }
}

double ConstraintEvaluator::getDynamicWeight(HardConstraintType type) const {
    auto it = dynamic_weights.find(type);
    return (it != dynamic_weights.end()) ? it->second : 1.0;
}

std::map<HardConstraintType, double> ConstraintEvaluator::getDynamicWeights() const {
    return dynamic_weights;
}

std::map<HardConstraintType, int> ConstraintEvaluator::getViolationCounts() const {
    return violation_counts;
}

double ConstraintEvaluator::getWeightedHardConstraintViolations(const Schedule& schedule) {
    double weighted_score = 0;
    
    // Apply dynamic weights to each constraint type
    weighted_score += getDynamicWeight(HardConstraintType::MAX_ONE_SHIFT_PER_DAY) * 
                     hard_constraints.evaluateMaxOneShiftPerDay(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::SHIFT_ROTATION) * 
                     hard_constraints.evaluateShiftRotation(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::MAX_SHIFTS_PER_TYPE) * 
                     hard_constraints.evaluateMaxShiftsPerType(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::WORKING_TIME_CONSTRAINTS) * 
                     hard_constraints.evaluateWorkingTimeConstraints(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::MAX_CONSECUTIVE_SHIFTS) * 
                     hard_constraints.evaluateMaxConsecutiveShifts(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::MIN_CONSECUTIVE_SHIFTS) * 
                     hard_constraints.evaluateMinConsecutiveShifts(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::MIN_CONSECUTIVE_DAYS_OFF) * 
                     hard_constraints.evaluateMinConsecutiveDaysOff(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::MAX_WEEKENDS_WORKED) * 
                     hard_constraints.evaluateMaxWeekendsWorked(schedule);
    weighted_score += getDynamicWeight(HardConstraintType::PRE_ASSIGNED_DAYS_OFF) * 
                     hard_constraints.evaluatePreAssignedDaysOff(schedule);
    
    return weighted_score;
}
