#ifndef INCREMENTAL_EVALUATOR_H
#define INCREMENTAL_EVALUATOR_H

#include "constraint_evaluator.h"
#include "../core/move.h"

class IncrementalEvaluator {
public:
    IncrementalEvaluator(ConstraintEvaluator& evaluator, const Schedule& initial_schedule);

    double getTotalScore() const;
    double getHardScore() const;
    double getSoftScore() const;
    Schedule getCurrentSchedule() const;
    void applyMove(const Move& move);
    double getDelta(const Move& move);
    void reset(const Schedule& schedule);

private:
    ConstraintEvaluator& evaluator;
    Schedule current_schedule;
    double current_hard_score;
    double current_soft_score;
};

#endif // INCREMENTAL_EVALUATOR_H
