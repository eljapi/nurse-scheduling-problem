#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "../core/instance.h"
#include "../core/data_structures.h"
#include "../constraints/incremental_evaluator.h"
#include "../core/move.h"
#include "neighborhood.h"

enum class SolveMode { Feasibility, Optimization };

class SimulatedAnnealing {
public:
    SimulatedAnnealing(const Instance& instance, ConstraintEvaluator& evaluator,
                       double initial_temp, double cooling, int max_iter, int stagnation);

    Schedule solve(SolveMode mode);

private:
    const Instance& instance;
    ConstraintEvaluator& evaluator;
    IncrementalEvaluator incremental_evaluator;
    Neighborhood neighborhood;

    // SA Parameters
    double initial_temperature;
    double cooling_rate;
    int max_iterations;
    int stagnation_limit;

    double acceptance(double delta, double temperature);
};

#endif // SIMULATED_ANNEALING_H
