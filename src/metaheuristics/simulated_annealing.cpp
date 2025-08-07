#include "simulated_annealing.h"
#include "neighborhood.h"
#include "../core/data_structures.h"
#include "../constraints/incremental_evaluator.h"
#include "../utils/random.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

SimulatedAnnealing::SimulatedAnnealing(const Instance& instance, ConstraintEvaluator& evaluator,
                                       double initial_temp, double cooling, int max_iter, int stagnation)
    : instance(instance),
      evaluator(evaluator),
      incremental_evaluator(evaluator, Schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes())),
      neighborhood(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes(), evaluator),
      initial_temperature(initial_temp),
      cooling_rate(cooling),
      max_iterations(max_iter),
      stagnation_limit(stagnation) {}

Schedule SimulatedAnnealing::solve(SolveMode mode) {
    Schedule current_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    current_schedule.randomize(instance.getNumShiftTypes());
    return solve(current_schedule, mode);
}

Schedule SimulatedAnnealing::solve(const Schedule& initial_schedule, SolveMode mode) {
    Schedule current_schedule = initial_schedule;

    incremental_evaluator.reset(current_schedule);

    Schedule best_schedule = current_schedule;
    double best_hard_score = incremental_evaluator.getHardScore();
    double best_soft_score = incremental_evaluator.getSoftScore();

    double temperature = initial_temperature;
    int stagnated = 0;

    for (int i = 0; i < max_iterations; ++i) {
        Move move = neighborhood.getRandomMove(current_schedule);
        
        double delta_hard = incremental_evaluator.getHardScoreDelta(move);
        double delta_soft = incremental_evaluator.getSoftScoreDelta(move);
        double new_hard_score = incremental_evaluator.getHardScore() + delta_hard;

        bool accept_move = false;
        double random_prob = Random::getDouble(0.0, 1.0);

        if (incremental_evaluator.getHardScore() < 0) {
            if (acceptance(delta_hard, temperature) > random_prob) {
                accept_move = true;
            }
        } else {
            if (new_hard_score < 0) {
                accept_move = false;
            } else {
                if (acceptance(delta_soft, temperature) > random_prob) {
                    accept_move = true;
                }
            }
        }

        if (accept_move) {
            incremental_evaluator.applyMove(move);
            current_schedule = incremental_evaluator.getCurrentSchedule();
        }

        if (incremental_evaluator.getHardScore() > best_hard_score ||
            (incremental_evaluator.getHardScore() == best_hard_score && incremental_evaluator.getSoftScore() > best_soft_score)) {
            best_schedule = current_schedule;
            best_hard_score = incremental_evaluator.getHardScore();
            best_soft_score = incremental_evaluator.getSoftScore();
            stagnated = 0;
        } else {
            stagnated++;
        }

        if (stagnated > stagnation_limit) {
            std::cout << "--- ESTANCAMIENTO DETECTADO! RECALENTANDO Y PERTURBANDO ---" << std::endl;
            
            current_schedule = best_schedule;
            
            neighborhood.perturb(current_schedule, 0.15);

            incremental_evaluator.reset(current_schedule);
            
            temperature = initial_temperature; 
            
            stagnated = 0;
        }

        const double MINIMUM_TEMPERATURE = 1e-8;

        temperature = std::max(temperature * cooling_rate, MINIMUM_TEMPERATURE);
        
        if (i % 100 == 0) {
            std::cout << "Iteration " << i << ": "
                      << "Best Hard Score = " << best_hard_score
                      << ", Best Soft Score = " << best_soft_score
                      << ", Current Hard Score = " << incremental_evaluator.getHardScore()
                      << ", Current Soft Score = " << incremental_evaluator.getSoftScore()
                      << ", Temperature = " << temperature << std::endl;
        }

        if (mode == SolveMode::Feasibility && best_hard_score == 0) {
            std::cout << "Feasible solution found!" << std::endl;
            return best_schedule;
        }
    }

    return best_schedule;
}

double SimulatedAnnealing::acceptance(double delta, double temperature) {
    if (delta > 0) {
        return 1.0;
    }
    if (temperature == 0) {
        return 0.0;
    }
    return exp(delta / temperature);
}
