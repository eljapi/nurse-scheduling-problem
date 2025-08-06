#include "incremental_evaluator.h"
#include "../core/data_structures.h"

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
    if (move.type == MoveType::Change || move.type == MoveType::FixShiftRotation) {
        current_schedule.setAssignment(move.employee1, move.day1, move.shift2);
    } else if (move.type == MoveType::Swap) {
        // This handles both regular swaps and the rebalance "transfer"
        current_schedule.setAssignment(move.employee1, move.day1, move.shift2);
        current_schedule.setAssignment(move.employee2, move.day2, move.shift1);
    } else if (move.type == MoveType::BlockSwap) {
        for (int d = 0; d < move.block_size; ++d) {
            int shift1 = current_schedule.getAssignment(move.employee1, move.day1 + d);
            int shift2 = current_schedule.getAssignment(move.employee2, move.day1 + d);
            current_schedule.setAssignment(move.employee1, move.day1 + d, shift2);
            current_schedule.setAssignment(move.employee2, move.day1 + d, shift1);
        }
    }
    // For RuinAndRecreate, the schedule is updated in the SA class, so we just reset
    reset(current_schedule);
}

double IncrementalEvaluator::getDelta(const Move& move) {
    Schedule temp_schedule = current_schedule;
    if (move.type == MoveType::Change || move.type == MoveType::FixShiftRotation) {
        temp_schedule.setAssignment(move.employee1, move.day1, move.shift2);
    } else if (move.type == MoveType::Swap) {
        temp_schedule.setAssignment(move.employee1, move.day1, move.shift2);
        temp_schedule.setAssignment(move.employee2, move.day2, move.shift1);
    } else if (move.type == MoveType::BlockSwap) {
        for (int d = 0; d < move.block_size; ++d) {
            int shift1 = temp_schedule.getAssignment(move.employee1, move.day1 + d);
            int shift2 = temp_schedule.getAssignment(move.employee2, move.day1 + d);
            temp_schedule.setAssignment(move.employee1, move.day1 + d, shift2);
            temp_schedule.setAssignment(move.employee2, move.day1 + d, shift1);
        }
    } else if (move.type == MoveType::RuinAndRecreate) {
        // Ruin
        for (int d = 0; d < temp_schedule.getHorizonDays(); ++d) {
            temp_schedule.setAssignment(move.employee1, d, 0); // Unassign
        }
        // Recreate
        for (int d = 0; d < temp_schedule.getHorizonDays(); ++d) {
            int best_shift = 0;
            double best_shift_score = -1e9;
            for (int s = 1; s <= temp_schedule.getNumShiftTypes(); ++s) {
                temp_schedule.setAssignment(move.employee1, d, s);
                double score = evaluator.evaluateSchedule(temp_schedule);
                if (score > best_shift_score) {
                    best_shift_score = score;
                    best_shift = s;
                }
            }
            temp_schedule.setAssignment(move.employee1, d, best_shift);
        }
    }

    double new_hard_score = evaluator.getHardConstraintViolations(temp_schedule);
    double new_soft_score = evaluator.getSoftConstraintViolations(temp_schedule);
    
    if (current_hard_score < 0 || new_hard_score < 0) {
        return new_hard_score - current_hard_score;
    } else {
        return new_soft_score - current_soft_score;
    }
}

void IncrementalEvaluator::reset(const Schedule& schedule) {
    current_schedule = schedule;
    current_hard_score = evaluator.getHardConstraintViolations(current_schedule);
    current_soft_score = evaluator.getSoftConstraintViolations(current_schedule);
}
