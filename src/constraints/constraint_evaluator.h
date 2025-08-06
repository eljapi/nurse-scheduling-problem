#ifndef CONSTRAINT_EVALUATOR_H
#define CONSTRAINT_EVALUATOR_H

#include "../core/instance.h"
#include "../core/data_structures.h"
#include "hard_constraints.h"
#include "soft_constraints.h"

class ConstraintEvaluator {
private:
    const Instance& instance;
    HardConstraints hard_constraints;
    SoftConstraints soft_constraints;

public:
    ConstraintEvaluator(const Instance& inst);
    double evaluateSchedule(const Schedule& schedule);
    bool isFeasible(const Schedule& schedule);
    double getHardConstraintViolations(const Schedule& schedule);
    double getSoftConstraintPenalties(const Schedule& schedule);
};

#endif // CONSTRAINT_EVALUATOR_H
