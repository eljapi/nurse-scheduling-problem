#ifndef CONSTRAINT_EVALUATOR_H
#define CONSTRAINT_EVALUATOR_H

#include "../core/instance.h"
#include "../core/data_structures.h"
#include "hard_constraints.h"
#include "soft_constraints.h"

class ConstraintEvaluator {
public:
    const Instance& instance;
private:
    HardConstraints hard_constraints;
    SoftConstraints soft_constraints;

public:
    ConstraintEvaluator(const Instance& inst);
    double evaluateSchedule(const Schedule& schedule);
    bool isFeasible(const Schedule& schedule);
    double getHardConstraintViolations(const Schedule& schedule);
    double getSoftConstraintViolations(const Schedule& schedule);
    double getEmployeeHardConstraintViolations(const Schedule& schedule, int employee_id);
    double getEmployeeSoftConstraintViolations(const Schedule& schedule, int employee_id);
    std::vector<std::pair<int, int>> getViolatingAssignments(const Schedule& schedule);
    std::map<std::string, int> getHardConstraintViolationsMap(const Schedule& schedule);
};

#endif // CONSTRAINT_EVALUATOR_H
