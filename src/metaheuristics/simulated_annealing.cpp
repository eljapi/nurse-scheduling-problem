#include "simulated_annealing.h"
#include "neighborhood.h"
#include "../core/data_structures.h"
#include "../constraints/incremental_evaluator.h"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

SimulatedAnnealing::SimulatedAnnealing(const Instance& instance, ConstraintEvaluator& evaluator,
                                       double initial_temp, double cooling, int max_iter, int stagnation)
    : instance(instance),
      evaluator(evaluator),
      incremental_evaluator(evaluator, Schedule(instance.getNumEmployees(), instance.getHorizonDays())),
      neighborhood(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes()),
      initial_temperature(initial_temp),
      cooling_rate(cooling),
      max_iterations(max_iter),
      stagnation_limit(stagnation) {}

Schedule SimulatedAnnealing::solve() {
    Schedule current_schedule(instance.getNumEmployees(), instance.getHorizonDays());
    current_schedule.randomize(instance.getNumShiftTypes());

    new (&incremental_evaluator) IncrementalEvaluator(evaluator, current_schedule);

    Schedule best_schedule = current_schedule;
    double best_score = incremental_evaluator.getTotalScore();

    double temperature = initial_temperature;
    int stagnated = 0;

    for (int i = 0; i < max_iterations; ++i) {
        Move move = neighborhood.getRandomMove(current_schedule);
        
        double delta = incremental_evaluator.getDelta(move);
        double current_score = incremental_evaluator.getTotalScore();
        double new_score = current_score + delta;

        if (acceptance(current_score, new_score, temperature) > ((double)rand() / RAND_MAX)) {
            incremental_evaluator.applyMove(move);
            current_schedule.setAssignment(move.employee_id, move.day, move.new_shift);
        }

        if (incremental_evaluator.getTotalScore() > best_score) {
            best_schedule = current_schedule;
            best_score = incremental_evaluator.getTotalScore();
            stagnated = 0;
        } else {
            stagnated++;
        }

        if (stagnated > stagnation_limit) {
            current_schedule = best_schedule;
            neighborhood.perturb(current_schedule, 0.20); // 20% perturbation
            new (&incremental_evaluator) IncrementalEvaluator(evaluator, current_schedule);
            temperature = initial_temperature;
            stagnated = 0;
        }

        temperature *= cooling_rate;
        
        if (i % 100 == 0) {
            std::cout << "Iteration " << i << ": "
                      << "Best Score = " << best_score
                      << ", Current Score = " << incremental_evaluator.getTotalScore()
                      << ", Temperature = " << temperature << std::endl;
        }

        if (best_score == 0) {
            std::cout << "Optimal solution found!" << std::endl;
            break;
        }
    }

    return best_schedule;
}

double SimulatedAnnealing::acceptance(double current_score, double new_score, double temperature) {
    if (new_score > current_score) {
        return 1.0;
    }
    if (temperature == 0) {
        return 0.0;
    }
    return exp((new_score - current_score) / temperature);
}
