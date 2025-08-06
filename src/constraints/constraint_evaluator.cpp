#include "constraint_evaluator.h"
#include <algorithm>
#include <sstream>

ConstraintEvaluator::ConstraintEvaluator(const Instance& inst) 
    : instance(inst), hard_constraints(inst), soft_constraints(inst) {}

// Main evaluation methods that match original main.cpp pattern

int ConstraintEvaluator::evaluateHardConstraints(const Schedule& schedule) const {
    // This matches the original hard constraint evaluation pattern in main.cpp
    // where multiple constraint functions were called and their scores summed
    return hard_constraints.evaluateAll(schedule);
}

int ConstraintEvaluator::evaluateSoftConstraints(const Schedule& schedule) const {
    // This matches the original fitness calculation in main.cpp
    // where ShiftOnRequest + ShiftOffRequest + SectionCover were summed
    return soft_constraints.evaluateAll(schedule);
}

double ConstraintEvaluator::evaluateTotal(const Schedule& schedule) const {
    // Combined evaluation: hard constraint penalties + soft constraint rewards
    int hard_score = evaluateHardConstraints(schedule);
    int soft_score = evaluateSoftConstraints(schedule);
    
    // In the original code, hard constraints were penalties (negative)
    // and soft constraints were rewards/penalties that formed the fitness
    return static_cast<double>(hard_score + soft_score);
}

bool ConstraintEvaluator::isFeasible(const Schedule& schedule) const {
    return hard_constraints.isFeasible(schedule);
}

// Move evaluation for optimization algorithms

double ConstraintEvaluator::evaluateMove(const Schedule& schedule, int employee, int day, 
                                        int old_shift, int new_shift) const {
    // Evaluate the combined impact of a move on both hard and soft constraints
    int hard_change = hard_constraints.evaluateMove(schedule, employee, day, old_shift, new_shift);
    int soft_change = soft_constraints.evaluateMove(schedule, employee, day, old_shift, new_shift);
    
    return static_cast<double>(hard_change + soft_change);
}

int ConstraintEvaluator::evaluateHardConstraintMove(const Schedule& schedule, int employee, int day, 
                                                   int old_shift, int new_shift) const {
    return hard_constraints.evaluateMove(schedule, employee, day, old_shift, new_shift);
}

int ConstraintEvaluator::evaluateSoftConstraintMove(const Schedule& schedule, int employee, int day, 
                                                   int old_shift, int new_shift) const {
    return soft_constraints.evaluateMove(schedule, employee, day, old_shift, new_shift);
}

// Analysis and reporting methods

std::map<std::string, double> ConstraintEvaluator::getDetailedEvaluation(const Schedule& schedule) const {
    std::map<std::string, double> evaluation;
    
    // Hard constraint breakdown
    evaluation["hard_total"] = static_cast<double>(evaluateHardConstraints(schedule));
    evaluation["hard_shift_rotation"] = static_cast<double>(hard_constraints.evaluateShiftRotation(schedule));
    evaluation["hard_max_shifts_per_type"] = static_cast<double>(hard_constraints.evaluateMaxShiftsPerType(schedule));
    evaluation["hard_working_time"] = static_cast<double>(hard_constraints.evaluateWorkingTimeConstraints(schedule));
    evaluation["hard_max_consecutive_shifts"] = static_cast<double>(hard_constraints.evaluateMaxConsecutiveShifts(schedule));
    evaluation["hard_min_consecutive_shifts"] = static_cast<double>(hard_constraints.evaluateMinConsecutiveShifts(schedule));
    evaluation["hard_min_consecutive_days_off"] = static_cast<double>(hard_constraints.evaluateMinConsecutiveDaysOff(schedule));
    evaluation["hard_max_weekends"] = static_cast<double>(hard_constraints.evaluateMaxWeekendsWorked(schedule));
    evaluation["hard_pre_assigned_days_off"] = static_cast<double>(hard_constraints.evaluatePreAssignedDaysOff(schedule));
    
    // Soft constraint breakdown
    auto soft_scores = soft_constraints.getDetailedScores(schedule);
    evaluation["soft_total"] = static_cast<double>(evaluateSoftConstraints(schedule));
    evaluation["soft_shift_on_requests"] = static_cast<double>(soft_scores["shift_on_requests"]);
    evaluation["soft_shift_off_requests"] = static_cast<double>(soft_scores["shift_off_requests"]);
    evaluation["soft_coverage_requirements"] = static_cast<double>(soft_scores["coverage_requirements"]);
    
    // Overall scores
    evaluation["total_score"] = evaluateTotal(schedule);
    evaluation["feasible"] = isFeasible(schedule) ? 1.0 : 0.0;
    
    return evaluation;
}

std::vector<std::string> ConstraintEvaluator::getViolationReport(const Schedule& schedule) const {
    std::vector<std::string> report;
    
    // Add hard constraint violations
    auto hard_violations = hard_constraints.getViolationDetails(schedule);
    for (const auto& violation : hard_violations) {
        report.push_back("[HARD] " + violation);
    }
    
    // Add soft constraint issues
    auto soft_issues = soft_constraints.getUnsatisfiedRequests(schedule);
    for (const auto& issue : soft_issues) {
        report.push_back("[SOFT] " + issue);
    }
    
    // Add summary
    std::ostringstream summary;
    summary << "Summary: " << hard_violations.size() << " hard violations, " 
            << soft_issues.size() << " soft issues";
    report.insert(report.begin(), summary.str());
    
    return report;
}

std::map<std::string, double> ConstraintEvaluator::getConstraintStatistics(const Schedule& schedule) const {
    std::map<std::string, double> stats;
    
    // Get hard constraint statistics
    auto hard_stats = hard_constraints.getConstraintStatistics(schedule);
    for (const auto& pair : hard_stats) {
        stats["hard_" + pair.first] = pair.second;
    }
    
    // Get soft constraint statistics
    auto soft_stats = soft_constraints.getSatisfactionRates(schedule);
    for (const auto& pair : soft_stats) {
        stats["soft_" + pair.first] = pair.second;
    }
    
    // Overall statistics
    stats["overall_feasibility"] = isFeasible(schedule) ? 1.0 : 0.0;
    stats["soft_satisfaction_percentage"] = getSoftConstraintSatisfactionRate(schedule);
    
    return stats;
}

// Access to individual constraint evaluators

const HardConstraints& ConstraintEvaluator::getHardConstraints() const {
    return hard_constraints;
}

const SoftConstraints& ConstraintEvaluator::getSoftConstraints() const {
    return soft_constraints;
}

// Utility methods for compatibility with original code

double ConstraintEvaluator::evaluateEmployee(const Schedule& schedule, int employee) const {
    int hard_score = hard_constraints.evaluateEmployee(schedule, employee);
    int soft_score = soft_constraints.evaluateEmployee(schedule, employee);
    
    return static_cast<double>(hard_score + soft_score);
}

int ConstraintEvaluator::getMaxPossibleSoftScore() const {
    return soft_constraints.getMaxPossibleScore();
}

double ConstraintEvaluator::getSoftConstraintSatisfactionRate(const Schedule& schedule) const {
    return soft_constraints.getSatisfactionPercentage(schedule);
}