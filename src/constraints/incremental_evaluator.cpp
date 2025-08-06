#include "incremental_evaluator.h"
#include "../core/data_structures.h"

IncrementalEvaluator::IncrementalEvaluator(ConstraintEvaluator& evaluator, const Schedule& initial_schedule)
    : evaluator(evaluator), current_schedule(initial_schedule) {
    current_score = evaluator.getHardConstraintViolations(current_schedule) + evaluator.getSoftConstraintViolations(current_schedule);
}

double IncrementalEvaluator::getTotalScore() const {
    return current_score;
}

void IncrementalEvaluator::applyMove(const Move& move) {
    current_score += getDelta(move);
    current_schedule.setAssignment(move.employee_id, move.day, move.new_shift);
}

double IncrementalEvaluator::getDelta(const Move& move) {
    double delta = 0;

    // Store the original shift
    int original_shift = current_schedule.getAssignment(move.employee_id, move.day);

    // Calculate the score before the move for the affected employee
    double old_employee_score = evaluator.getEmployeeHardConstraintViolations(current_schedule, move.employee_id) +
                                evaluator.getEmployeeSoftConstraintViolations(current_schedule, move.employee_id);

    // Create a temporary schedule to evaluate the new score
    Schedule temp_schedule = current_schedule;
    temp_schedule.setAssignment(move.employee_id, move.day, move.new_shift);

    // Calculate the score after the move for the affected employee
    double new_employee_score = evaluator.getEmployeeHardConstraintViolations(temp_schedule, move.employee_id) +
                                evaluator.getEmployeeSoftConstraintViolations(temp_schedule, move.employee_id);

    delta = new_employee_score - old_employee_score;

    return delta;
}
