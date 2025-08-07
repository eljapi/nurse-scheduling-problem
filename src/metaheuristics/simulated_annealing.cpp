#include "simulated_annealing.h"
#include "neighborhood.h"
#include "../core/data_structures.h"
#include "../constraints/incremental_evaluator.h"
#include "../utils/random.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <sstream>

// TabuMemory implementation
void TabuMemory::addMove(int employee, int day, int shift) {
    std::pair<int, int> move_pair = {employee, day};
    recent_moves.push_back(move_pair);
    
    std::string assignment_key = getAssignmentKey(employee, day, shift);
    tabu_assignments.insert(assignment_key);
    
    if (recent_moves.size() > max_size) {
        auto old_move = recent_moves.front();
        recent_moves.pop_front();
        
        // Remove old tabu assignments (simplified - in practice might need more sophisticated cleanup)
        if (tabu_assignments.size() > max_size * 2) {
            tabu_assignments.clear();
            // Re-add recent assignments
            for (const auto& move : recent_moves) {
                std::string key = getAssignmentKey(move.first, move.second, 0); // Simplified
                tabu_assignments.insert(key);
            }
        }
    }
}

bool TabuMemory::isTabu(int employee, int day, int shift) const {
    std::string assignment_key = getAssignmentKey(employee, day, shift);
    return tabu_assignments.find(assignment_key) != tabu_assignments.end();
}

void TabuMemory::clear() {
    recent_moves.clear();
    tabu_assignments.clear();
}

std::string TabuMemory::getAssignmentKey(int employee, int day, int shift) const {
    std::ostringstream oss;
    oss << employee << "_" << day << "_" << shift;
    return oss.str();
}

// DiversificationIntensification implementation
DiversificationIntensification::DiversificationIntensification(const Instance& instance, ConstraintEvaluator& evaluator)
    : instance(instance), evaluator(evaluator), 
      neighborhood(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes(), evaluator) {}

Schedule DiversificationIntensification::diversifyRestart(const Schedule& current_best, double perturbation_rate) {
    Schedule diversified = current_best;
    neighborhood.perturb(diversified, perturbation_rate);
    return diversified;
}

Schedule DiversificationIntensification::diversifyRandomRestart(const Schedule& current_best) {
    Schedule random_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    random_schedule.randomize(instance.getNumShiftTypes());
    return random_schedule;
}

Schedule DiversificationIntensification::diversifyGuidedRestart(const Schedule& current_best) {
    Schedule guided = current_best;
    
    // Apply guided perturbation focusing on constraint violations
    std::vector<std::pair<int, int>> violations = evaluator.getViolatingAssignments(current_best);
    
    for (const auto& violation : violations) {
        int employee = violation.first;
        int day = violation.second;
        
        // Try to fix this violation by changing to a day off or different shift
        if (Random::getDouble(0.0, 1.0) < 0.7) { // 70% chance to fix
            int current_shift = guided.getAssignment(employee, day);
            int new_shift = (current_shift == 0) ? Random::getInt(1, instance.getNumShiftTypes()) : 0;
            guided.setAssignment(employee, day, new_shift);
        }
    }
    
    return guided;
}

Schedule DiversificationIntensification::intensifyLocalSearch(const Schedule& schedule, int max_iterations) {
    Schedule current = schedule;
    Schedule best = schedule;
    double best_score = evaluator.evaluateSchedule(schedule);
    
    for (int i = 0; i < max_iterations; ++i) {
        Move move = neighborhood.getRandomMove(current);
        
        // Apply move temporarily to evaluate
        Schedule temp = current;
        if (move.type == MoveType::Change) {
            temp.setAssignment(move.employee1, move.day1, move.shift2);
        } else if (move.type == MoveType::Swap) {
            temp.setAssignment(move.employee1, move.day1, move.shift2);
            temp.setAssignment(move.employee2, move.day2, move.shift1);
        }
        
        double new_score = evaluator.evaluateSchedule(temp);
        
        // Accept only improving moves (hill climbing)
        if (new_score > best_score) {
            current = temp;
            best = temp;
            best_score = new_score;
        }
    }
    
    return best;
}

Schedule DiversificationIntensification::intensifyHillClimbing(const Schedule& schedule, int max_iterations) {
    return intensifyLocalSearch(schedule, max_iterations); // Same implementation for now
}

Schedule DiversificationIntensification::intensifyVariableNeighborhood(const Schedule& schedule, int max_iterations) {
    Schedule current = schedule;
    Schedule best = schedule;
    double best_score = evaluator.evaluateSchedule(schedule);
    
    std::vector<int> neighborhood_types = {0, 1, 2, 3}; // Different move types
    
    for (int i = 0; i < max_iterations; ++i) {
        bool improved = false;
        
        // Try different neighborhood structures
        for (int nh_type : neighborhood_types) {
            Move move;
            
            // Generate move based on neighborhood type
            switch (nh_type) {
                case 0: move = neighborhood.getRandomMove(current); break;
                case 1: move = neighborhood.getRandomMove(current); break; // Could be different strategies
                case 2: move = neighborhood.getRandomMove(current); break;
                case 3: move = neighborhood.getRandomMove(current); break;
            }
            
            // Apply move temporarily
            Schedule temp = current;
            if (move.type == MoveType::Change) {
                temp.setAssignment(move.employee1, move.day1, move.shift2);
            } else if (move.type == MoveType::Swap) {
                temp.setAssignment(move.employee1, move.day1, move.shift2);
                temp.setAssignment(move.employee2, move.day2, move.shift1);
            }
            
            double new_score = evaluator.evaluateSchedule(temp);
            
            if (new_score > best_score) {
                current = temp;
                best = temp;
                best_score = new_score;
                improved = true;
                break; // Move to next iteration with first neighborhood that improves
            }
        }
        
        if (!improved) {
            break; // No improvement found in any neighborhood
        }
    }
    
    return best;
}

SimulatedAnnealing::SimulatedAnnealing(const Instance& instance, ConstraintEvaluator& evaluator,
                                       double initial_temp, double cooling, int max_iter, int stagnation)
    : instance(instance),
      evaluator(evaluator),
      incremental_evaluator(evaluator, Schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes())),
      neighborhood(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes(), evaluator),
      tabu_memory(50), // Tabu memory size
      div_int_strategies(instance, evaluator),
      initial_temperature(initial_temp),
      cooling_rate(cooling),
      max_iterations(max_iter),
      stagnation_limit(stagnation),
      restart_count(0),
      max_restarts(5),
      intensification_frequency(200),
      diversification_frequency(500),
      elite_size(5) {}

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

    // Initialize elite solutions
    updateEliteSolutions(best_schedule, best_hard_score, best_soft_score);

    double temperature = initial_temperature;
    int stagnated = 0;
    int iterations_since_improvement = 0;
    restart_count = 0;

    for (int i = 0; i < max_iterations; ++i) {
        // Check for diversification/intensification triggers
        if (shouldDiversify(iterations_since_improvement) && restart_count < max_restarts) {
            std::cout << "--- DIVERSIFICATION: Applying guided restart ---" << std::endl;
            
            Schedule diversification_base = selectDiversificationBase();
            current_schedule = div_int_strategies.diversifyGuidedRestart(diversification_base);
            
            incremental_evaluator.reset(current_schedule);
            temperature = initial_temperature * 0.8; // Slightly lower temperature after restart
            tabu_memory.clear();
            stagnated = 0;
            iterations_since_improvement = 0;
            restart_count++;
            continue;
        }
        
        if (shouldIntensify(iterations_since_improvement)) {
            std::cout << "--- INTENSIFICATION: Applying local search ---" << std::endl;
            
            Schedule intensified = div_int_strategies.intensifyVariableNeighborhood(best_schedule, 50);
            double intensified_score = evaluator.evaluateSchedule(intensified);
            double current_best_score = evaluator.evaluateSchedule(best_schedule);
            
            if (intensified_score > current_best_score) {
                best_schedule = intensified;
                current_schedule = intensified;
                incremental_evaluator.reset(current_schedule);
                
                best_hard_score = evaluator.getHardConstraintViolations(best_schedule);
                best_soft_score = evaluator.getSoftConstraintViolations(best_schedule);
                updateEliteSolutions(best_schedule, best_hard_score, best_soft_score);
                
                iterations_since_improvement = 0;
                stagnated = 0;
            }
        }

        Move move = neighborhood.getRandomMove(current_schedule);
        
        // Check tabu status (with aspiration criteria)
        bool is_tabu = false;
        if (move.type == MoveType::Change) {
            is_tabu = tabu_memory.isTabu(move.employee1, move.day1, move.shift2);
        }
        
        double delta_hard = incremental_evaluator.getHardScoreDelta(move);
        double delta_soft = incremental_evaluator.getSoftScoreDelta(move);
        double new_hard_score = incremental_evaluator.getHardScore() + delta_hard;
        double new_soft_score = incremental_evaluator.getSoftScore() + delta_soft;

        bool accept_move = false;
        double random_prob = Random::getDouble(0.0, 1.0);

        // Aspiration criteria: accept tabu moves if they lead to new best solution
        bool aspiration = is_tabu && (new_hard_score > best_hard_score || 
                                     (new_hard_score == best_hard_score && new_soft_score > best_soft_score));

        if (!is_tabu || aspiration) {
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
        }

        if (accept_move) {
            incremental_evaluator.applyMove(move);
            current_schedule = incremental_evaluator.getCurrentSchedule();
            
            // Add move to tabu memory
            if (move.type == MoveType::Change) {
                tabu_memory.addMove(move.employee1, move.day1, move.shift1); // Add the old assignment as tabu
            }
        }

        // Update best solution
        if (incremental_evaluator.getHardScore() > best_hard_score ||
            (incremental_evaluator.getHardScore() == best_hard_score && incremental_evaluator.getSoftScore() > best_soft_score)) {
            
            best_schedule = current_schedule;
            best_hard_score = incremental_evaluator.getHardScore();
            best_soft_score = incremental_evaluator.getSoftScore();
            updateEliteSolutions(best_schedule, best_hard_score, best_soft_score);
            
            stagnated = 0;
            iterations_since_improvement = 0;
        } else {
            stagnated++;
            iterations_since_improvement++;
        }

        // Traditional restart mechanism (fallback)
        if (stagnated > stagnation_limit) {
            std::cout << "--- TRADITIONAL RESTART: Reheating and perturbing ---" << std::endl;
            
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
                      << ", Temperature = " << temperature
                      << ", Restarts = " << restart_count
                      << ", Elite Size = " << elite_solutions.size() << std::endl;
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

bool SimulatedAnnealing::shouldDiversify(int iterations_since_improvement) {
    return iterations_since_improvement > diversification_frequency;
}

bool SimulatedAnnealing::shouldIntensify(int iterations_since_improvement) {
    return iterations_since_improvement > 0 && iterations_since_improvement % intensification_frequency == 0;
}

void SimulatedAnnealing::updateEliteSolutions(const Schedule& schedule, double hard_score, double soft_score) {
    // Add to elite solutions if it's better than the worst elite or if we have space
    bool should_add = false;
    
    if (elite_solutions.size() < elite_size) {
        should_add = true;
    } else {
        // Find the worst elite solution
        double worst_hard = evaluator.getHardConstraintViolations(elite_solutions[0]);
        double worst_soft = evaluator.getSoftConstraintViolations(elite_solutions[0]);
        int worst_index = 0;
        
        for (int i = 1; i < elite_solutions.size(); ++i) {
            double curr_hard = evaluator.getHardConstraintViolations(elite_solutions[i]);
            double curr_soft = evaluator.getSoftConstraintViolations(elite_solutions[i]);
            
            if (curr_hard < worst_hard || (curr_hard == worst_hard && curr_soft < worst_soft)) {
                worst_hard = curr_hard;
                worst_soft = curr_soft;
                worst_index = i;
            }
        }
        
        // Replace if current solution is better than worst elite
        if (hard_score > worst_hard || (hard_score == worst_hard && soft_score > worst_soft)) {
            elite_solutions[worst_index] = schedule;
        }
    }
    
    if (should_add) {
        elite_solutions.push_back(schedule);
    }
}

Schedule SimulatedAnnealing::selectDiversificationBase() {
    if (elite_solutions.empty()) {
        // Fallback to random schedule
        Schedule random_schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
        random_schedule.randomize(instance.getNumShiftTypes());
        return random_schedule;
    }
    
    // Select a random elite solution as base for diversification
    int random_index = Random::getInt(0, elite_solutions.size() - 1);
    return elite_solutions[random_index];
}
