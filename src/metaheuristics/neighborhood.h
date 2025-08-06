#ifndef NEIGHBORHOOD_H
#define NEIGHBORHOOD_H

#include "../core/data_structures.h"
#include "../core/move.h"
#include "../constraints/constraint_evaluator.h"

class Neighborhood {
public:
    Neighborhood(int num_employees, int horizon, int num_shift_types, ConstraintEvaluator& evaluator);

    Move getRandomMove(const Schedule& schedule);
    void perturb(Schedule& schedule, double rate);

private:
    Move getRandomChangeMove(const Schedule& schedule);
    Move getRandomSwapMove(const Schedule& schedule);
    Move getRandomBlockSwapMove(const Schedule& schedule);
    Move getRandomRuinAndRecreateMove(const Schedule& schedule);
    Move generateFixShiftRotationMove(const Schedule& schedule);
    Move findFixShiftRotationMove(const Schedule& schedule);
    Move generateFixHardConstraintMove(const Schedule& schedule);
    Move generateBalanceWorkingTimeMove(const Schedule& schedule);
    Move generateFixMaxConsecutiveShiftsMove(const Schedule& schedule);
    Move generateFixMinConsecutiveShiftsMove(const Schedule& schedule);
    Move generateFixMinConsecutiveDaysOffMove(const Schedule& schedule);
    Move generateRebalanceWorkloadMove(const Schedule& schedule);
    Move generateConsolidateWorkMove(const Schedule& schedule);

    int num_employees;
    int horizon;
    int num_shift_types;
    ConstraintEvaluator& evaluator;
};

#endif // NEIGHBORHOOD_H
