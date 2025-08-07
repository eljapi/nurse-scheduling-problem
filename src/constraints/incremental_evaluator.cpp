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

double IncrementalEvaluator::getHardScoreDelta(const Move& move) {
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
    }
    double new_hard_score = evaluator.getHardConstraintViolations(temp_schedule);
    return new_hard_score - current_hard_score;
}

double IncrementalEvaluator::getSoftScoreDelta(const Move& move) {
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
    }
    double new_soft_score = evaluator.getSoftConstraintViolations(temp_schedule);
    return new_soft_score - current_soft_score;
}

void IncrementalEvaluator::reset(const Schedule& schedule) {
    current_schedule = schedule;
    current_hard_score = evaluator.getHardConstraintViolations(current_schedule);
    current_soft_score = evaluator.getSoftConstraintViolations(current_schedule);
}
