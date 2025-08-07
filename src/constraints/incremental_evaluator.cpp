#include "incremental_evaluator.h"
#include "../core/data_structures.h"
#include <iostream>
#include <cmath>

IncrementalEvaluator::IncrementalEvaluator(ConstraintEvaluator& evaluator, const Schedule& initial_schedule)
    : evaluator(evaluator), current_schedule(initial_schedule) {
    reset(initial_schedule);
}

double IncrementalEvaluator::getTotalScore() const {
    return current_hard_score + current_soft_score;
}

double IncrementalEvaluator::getHardScore() const {
    return current_hard_score;
}

double IncrementalEvaluator::getSoftScore() const {
    return current_soft_score;
}

Schedule IncrementalEvaluator::getCurrentSchedule() const {
    return current_schedule;
}

void IncrementalEvaluator::applyMove(const Move& move) {
    // Calculate deltas BEFORE modifying the schedule
    double delta_hard = getHardScoreDelta(move);
    double delta_soft = getSoftScoreDelta(move);
    
    // Apply the move to the schedule
    if (move.type == MoveType::Change || move.type == MoveType::FixShiftRotation) {
        current_schedule.setAssignment(move.employee1, move.day1, move.shift2);
    } else if (move.type == MoveType::Swap) {
        int original_shift1 = current_schedule.getAssignment(move.employee1, move.day1);
        int original_shift2 = current_schedule.getAssignment(move.employee2, move.day2);
        current_schedule.setAssignment(move.employee1, move.day1, original_shift2);
        current_schedule.setAssignment(move.employee2, move.day2, original_shift1);
    } else if (move.type == MoveType::BlockSwap) {
        for (int d = 0; d < move.block_size; ++d) {
            int day = move.day1 + d;
            int shift1 = current_schedule.getAssignment(move.employee1, day);
            int shift2 = current_schedule.getAssignment(move.employee2, day);
            current_schedule.setAssignment(move.employee1, day, shift2);
            current_schedule.setAssignment(move.employee2, day, shift1);
        }
    } else if (move.type == MoveType::RuinAndRecreate) {
        reset(current_schedule);
        return;
    }
    
    // Update scores incrementally
    current_hard_score += delta_hard;
    current_soft_score += delta_soft;
}

double IncrementalEvaluator::getHardScoreDelta(const Move& move) {
    double total_delta = 0.0;
    
    switch (move.type) {
        case MoveType::Change:
        case MoveType::FixShiftRotation: {
            total_delta = evaluator.hard_constraints.calculateEmployeeDelta(current_schedule, move.employee1, move.day1, move.shift2);
            break;
        }
        case MoveType::Swap: {
            // For swaps, we need to be careful about the order and what shifts we're swapping
            int current_shift1 = current_schedule.getAssignment(move.employee1, move.day1);
            int current_shift2 = current_schedule.getAssignment(move.employee2, move.day2);
            
            // Employee1 changes from current_shift1 to current_shift2
            double delta1 = evaluator.hard_constraints.calculateEmployeeDelta(current_schedule, move.employee1, move.day1, current_shift2);
            
            // Employee2 changes from current_shift2 to current_shift1
            // We need to calculate this after the first change to account for interactions
            Schedule temp_schedule = current_schedule;
            temp_schedule.setAssignment(move.employee1, move.day1, current_shift2);
            double delta2 = evaluator.hard_constraints.calculateEmployeeDelta(temp_schedule, move.employee2, move.day2, current_shift1);
            
            total_delta = delta1 + delta2;
            break;
        }
        case MoveType::BlockSwap: {
            Schedule temp_schedule = current_schedule;
            double old_total_score = evaluator.getEmployeeHardConstraintViolations(temp_schedule, move.employee1) +
                                   evaluator.getEmployeeHardConstraintViolations(temp_schedule, move.employee2);
            
            for (int d = 0; d < move.block_size; ++d) {
                int day = move.day1 + d;
                int shift1 = temp_schedule.getAssignment(move.employee1, day);
                int shift2 = temp_schedule.getAssignment(move.employee2, day);
                temp_schedule.setAssignment(move.employee1, day, shift2);
                temp_schedule.setAssignment(move.employee2, day, shift1);
            }
            
            double new_total_score = evaluator.getEmployeeHardConstraintViolations(temp_schedule, move.employee1) +
                                   evaluator.getEmployeeHardConstraintViolations(temp_schedule, move.employee2);
            total_delta = new_total_score - old_total_score;
            break;
        }
        case MoveType::RuinAndRecreate: {
            // For RuinAndRecreate, we still need full re-evaluation
            Schedule temp_schedule = current_schedule;
            double new_hard_score = evaluator.getHardConstraintViolations(temp_schedule);
            total_delta = new_hard_score - current_hard_score;
            break;
        }
    }
    
    return total_delta;
}

double IncrementalEvaluator::getSoftScoreDelta(const Move& move) {
    double total_delta = 0.0;
    
    switch (move.type) {
        case MoveType::Change:
        case MoveType::FixShiftRotation: {
            total_delta += evaluator.soft_constraints.calculateEmployeeDelta(current_schedule, move.employee1, move.day1, move.shift2);
            total_delta += evaluator.soft_constraints.calculateCoverageDelta(current_schedule, move.day1, move.shift1, move.shift2);
            break;
        }
        case MoveType::Swap: {
            // Get the current shifts that will be swapped
            int current_shift1 = current_schedule.getAssignment(move.employee1, move.day1);
            int current_shift2 = current_schedule.getAssignment(move.employee2, move.day2);
            
            // Employee preference deltas
            total_delta += evaluator.soft_constraints.calculateEmployeeDelta(current_schedule, move.employee1, move.day1, current_shift2);
            total_delta += evaluator.soft_constraints.calculateEmployeeDelta(current_schedule, move.employee2, move.day2, current_shift1);
            
            if (move.day1 == move.day2) {
                // Same day swap - coverage changes cancel out if shifts are different
                if (current_shift1 != current_shift2) {
                    // No net coverage change for same-day swaps of different shifts
                    // The coverage delta is 0 because we're just moving people around
                }
            } else {
                // Different days - calculate coverage impact for each day
                total_delta += evaluator.soft_constraints.calculateCoverageDelta(current_schedule, move.day1, current_shift1, current_shift2);
                total_delta += evaluator.soft_constraints.calculateCoverageDelta(current_schedule, move.day2, current_shift2, current_shift1);
            }
            break;
        }
        case MoveType::BlockSwap: {
            Schedule temp_schedule = current_schedule;
            double old_total_score = evaluator.getSoftConstraintViolations(temp_schedule);
            
            for (int d = 0; d < move.block_size; ++d) {
                int day = move.day1 + d;
                int shift1 = temp_schedule.getAssignment(move.employee1, day);
                int shift2 = temp_schedule.getAssignment(move.employee2, day);
                temp_schedule.setAssignment(move.employee1, day, shift2);
                temp_schedule.setAssignment(move.employee2, day, shift1);
            }
            
            double new_total_score = evaluator.getSoftConstraintViolations(temp_schedule);
            total_delta = new_total_score - old_total_score;
            break;
        }
        case MoveType::RuinAndRecreate: {
            // For RuinAndRecreate, we still need full re-evaluation
            Schedule temp_schedule = current_schedule;
            double new_soft_score = evaluator.getSoftConstraintViolations(temp_schedule);
            total_delta = new_soft_score - current_soft_score;
            break;
        }
    }
    
    return total_delta;
}

void IncrementalEvaluator::reset(const Schedule& schedule) {
    current_schedule = schedule;
    current_hard_score = evaluator.getHardConstraintViolations(current_schedule);
    current_soft_score = evaluator.getSoftConstraintViolations(current_schedule);
}
