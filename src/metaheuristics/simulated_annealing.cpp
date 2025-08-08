#include "simulated_annealing.h"
#include "neighborhood.h"
#include "initial_solution.h"
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
    // Use feasible initial solution instead of random initialization
    InitialSolutionGenerator generator(instance);
    Schedule feasible_schedule = generator.generateFeasibleSolution();
    return feasible_schedule;
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
                                       double initial_temp, double cooling, int max_iter, int stagnation,
                                       int weight_update_freq)
    : instance(instance),
      evaluator(evaluator),
      incremental_evaluator(evaluator, Schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes())),
      neighborhood(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes(), evaluator),
      tabu_memory(50), // Tabu memory size
      div_int_strategies(instance, evaluator),
      initial_solution_generator(instance),
      initial_temperature(initial_temp),
      cooling_rate(cooling),
      max_iterations(max_iter),
      stagnation_limit(stagnation),
      weight_update_frequency(weight_update_freq == -1 ? stagnation / 2 : weight_update_freq),
      restart_count(0),
      max_restarts(5),
      intensification_frequency(200),
      diversification_frequency(500),
      elite_size(5),
      weighted_moves_accepted(0),
      total_moves_evaluated(0) {}

Schedule SimulatedAnnealing::solve(SolveMode mode) {
    // Use the 5-step feasible initial solution heuristic instead of random initialization
    std::cout << "Generating feasible initial solution using 5-step heuristic..." << std::endl;
    Schedule current_schedule = initial_solution_generator.generateFeasibleSolution();
    std::cout << "Initial solution generated. Starting simulated annealing..." << std::endl;
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
    int iterations_since_weight_update = 0;
    restart_count = 0;

    for (int i = 0; i < max_iterations; ++i) {
        // Update dynamic weights periodically based on violation patterns
        if (iterations_since_weight_update >= weight_update_frequency) {
            evaluator.updateDynamicWeights(current_schedule);
            iterations_since_weight_update = 0;
            
            if (i % 1000 == 0) { // Log weight updates occasionally
                std::cout << "--- WEIGHT UPDATE at iteration " << i << " ---" << std::endl;
                auto weights = evaluator.getDynamicWeights();
                auto violations = evaluator.getViolationCounts();
                
                // Show current constraint violations
                double regular_hard = evaluator.getHardConstraintViolations(current_schedule);
                double weighted_hard = evaluator.getWeightedHardConstraintViolations(current_schedule);
                
                std::cout << "  Regular hard score: " << regular_hard << ", Weighted: " << weighted_hard << std::endl;
                
                for (const auto& pair : weights) {
                    HardConstraintType constraint_type = pair.first;
                    std::string constraint_name;
                    switch (constraint_type) {
                        case HardConstraintType::MAX_ONE_SHIFT_PER_DAY: constraint_name = "MaxOneShift"; break;
                        case HardConstraintType::SHIFT_ROTATION: constraint_name = "ShiftRotation"; break;
                        case HardConstraintType::MAX_SHIFTS_PER_TYPE: constraint_name = "MaxShiftsPerType"; break;
                        case HardConstraintType::WORKING_TIME_CONSTRAINTS: constraint_name = "WorkingTime"; break;
                        case HardConstraintType::MAX_CONSECUTIVE_SHIFTS: constraint_name = "MaxConsecutive"; break;
                        case HardConstraintType::MIN_CONSECUTIVE_SHIFTS: constraint_name = "MinConsecutive"; break;
                        case HardConstraintType::MIN_CONSECUTIVE_DAYS_OFF: constraint_name = "MinDaysOff"; break;
                        case HardConstraintType::MAX_WEEKENDS_WORKED: constraint_name = "MaxWeekends"; break;
                        case HardConstraintType::PRE_ASSIGNED_DAYS_OFF: constraint_name = "PreAssigned"; break;
                        default: constraint_name = "Unknown"; break;
                    }
                    
                    if (violations.at(pair.first) > 0) {
                        std::cout << "  " << constraint_name << ": weight=" << pair.second 
                                  << ", violations=" << violations.at(pair.first) << std::endl;
                    }
                }
            }
        }
        // Check for diversification/intensification triggers
        if (shouldDiversify(iterations_since_improvement) && restart_count < max_restarts) {
            std::cout << "--- DIVERSIFICATION: Applying guided restart ---" << std::endl;
            
            Schedule diversification_base = selectDiversificationBase();
            current_schedule = div_int_strategies.diversifyGuidedRestart(diversification_base);
            
            incremental_evaluator.reset(current_schedule);
            temperature = initial_temperature * 0.8; // Slightly lower temperature after restart
            tabu_memory.clear();
            evaluator.resetDynamicWeights(); // Reset weights for fresh start
            stagnated = 0;
            iterations_since_improvement = 0;
            iterations_since_weight_update = 0;
            restart_count++;
            continue;
        }
        
        if (shouldIntensify(iterations_since_improvement)) {
            std::cout << "--- INTENSIFICATION: Applying local search on elite solutions ---" << std::endl;
            
            // Try intensification on multiple elite solutions, not just the best
            Schedule best_intensified = best_schedule;
            double best_intensified_score = evaluator.evaluateSchedule(best_schedule);
            
            // Intensify around each elite solution
            for (const auto& elite : elite_solutions) {
                Schedule intensified = div_int_strategies.intensifyVariableNeighborhood(elite, 30);
                double intensified_score = evaluator.evaluateSchedule(intensified);
                
                if (intensified_score > best_intensified_score) {
                    best_intensified = intensified;
                    best_intensified_score = intensified_score;
                }
            }
            
            if (best_intensified_score > evaluator.evaluateSchedule(best_schedule)) {
                best_schedule = best_intensified;
                current_schedule = best_intensified;
                incremental_evaluator.reset(current_schedule);
                
                best_hard_score = evaluator.getHardConstraintViolations(best_schedule);
                best_soft_score = evaluator.getSoftConstraintViolations(best_schedule);
                updateEliteSolutions(best_schedule, best_hard_score, best_soft_score);
                
                iterations_since_improvement = 0;
                stagnated = 0;
                
                std::cout << "  Elite-based intensification improved solution!" << std::endl;
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
        
        // For infeasible solutions, use weighted evaluation to guide search
        double delta_to_use = delta_hard;
        if (incremental_evaluator.getHardScore() < 0) {
            // Use weighted evaluation for better constraint handling in infeasible region
            double current_weighted_hard = evaluator.getWeightedHardConstraintViolations(current_schedule);
            
            // Create temporary schedule to evaluate weighted score after move
            Schedule temp_schedule = current_schedule;
            if (move.type == MoveType::Change) {
                temp_schedule.setAssignment(move.employee1, move.day1, move.shift2);
            } else if (move.type == MoveType::Swap) {
                temp_schedule.setAssignment(move.employee1, move.day1, move.shift2);
                temp_schedule.setAssignment(move.employee2, move.day2, move.shift1);
            }
            double new_weighted_hard = evaluator.getWeightedHardConstraintViolations(temp_schedule);
            delta_to_use = new_weighted_hard - current_weighted_hard;
        }

        bool accept_move = false;
        double random_prob = Random::getDouble(0.0, 1.0);

        // Aspiration criteria: accept tabu moves if they lead to new best solution
        // For infeasible solutions, use weighted scores for aspiration too
        bool aspiration = false;
        if (is_tabu) {
            if (incremental_evaluator.getHardScore() < 0) {
                // In infeasible region, use weighted evaluation for aspiration
                double current_weighted = evaluator.getWeightedHardConstraintViolations(current_schedule);
                Schedule temp_schedule = current_schedule;
                if (move.type == MoveType::Change) {
                    temp_schedule.setAssignment(move.employee1, move.day1, move.shift2);
                } else if (move.type == MoveType::Swap) {
                    temp_schedule.setAssignment(move.employee1, move.day1, move.shift2);
                    temp_schedule.setAssignment(move.employee2, move.day2, move.shift1);
                }
                double new_weighted = evaluator.getWeightedHardConstraintViolations(temp_schedule);
                double best_weighted = evaluator.getWeightedHardConstraintViolations(best_schedule);
                aspiration = (new_weighted > best_weighted);
            } else {
                // In feasible region, use regular scores
                aspiration = (new_hard_score > best_hard_score || 
                             (new_hard_score == best_hard_score && new_soft_score > best_soft_score));
            }
        }

        total_moves_evaluated++;
        
        if (!is_tabu || aspiration) {
            if (incremental_evaluator.getHardScore() < 0) {
                // Use weighted delta for better constraint handling in infeasible region
                double acceptance_prob = acceptance(delta_to_use, temperature);
                double regular_acceptance_prob = acceptance(delta_hard, temperature);
                
                if (acceptance_prob > random_prob) {
                    accept_move = true;
                    
                    // Track when weighted evaluation makes a difference
                    if (std::abs(delta_to_use - delta_hard) > 0.1) {
                        weighted_moves_accepted++;
                        
                        // Debug: Show when weighted evaluation makes a difference
                        if (i % 10000 == 0) {
                            std::cout << "  Weighted delta (" << delta_to_use 
                                      << ") vs Regular delta (" << delta_hard 
                                      << ") - Weighted prob: " << acceptance_prob 
                                      << ", Regular prob: " << regular_acceptance_prob << std::endl;
                        }
                    }
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

        // Update best solution - use weighted evaluation when in infeasible region
        bool is_new_best = false;
        
        if (best_hard_score < 0 && incremental_evaluator.getHardScore() < 0) {
            // Both current and best are infeasible - compare using weighted scores
            double current_weighted = evaluator.getWeightedHardConstraintViolations(current_schedule);
            double best_weighted = evaluator.getWeightedHardConstraintViolations(best_schedule);
            is_new_best = (current_weighted > best_weighted);
        } else {
            // At least one is feasible - use regular comparison
            is_new_best = (incremental_evaluator.getHardScore() > best_hard_score ||
                          (incremental_evaluator.getHardScore() == best_hard_score && 
                           incremental_evaluator.getSoftScore() > best_soft_score));
        }
        
        if (is_new_best) {
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
        
        iterations_since_weight_update++;

        // Traditional restart mechanism (fallback)
        if (stagnated > stagnation_limit) {
            // Try path relinking with elite solutions first
            if (elite_solutions.size() >= 2 && Random::getDouble(0.0, 1.0) < 0.5) {
                std::cout << "--- PATH RELINKING: Combining elite solutions ---" << std::endl;
                current_schedule = pathRelinkingWithElites();
            } else {
                std::cout << "--- TRADITIONAL RESTART: Reheating and perturbing ---" << std::endl;
                current_schedule = best_schedule;
                neighborhood.perturb(current_schedule, 0.15);
            }
            
            incremental_evaluator.reset(current_schedule);
            temperature = initial_temperature;
            evaluator.resetDynamicWeights(); // Reset weights for traditional restart
            stagnated = 0;
            iterations_since_weight_update = 0;
        }

        const double MINIMUM_TEMPERATURE = 1e-8;
        temperature = std::max(temperature * cooling_rate, MINIMUM_TEMPERATURE);
        
        if (i % 100 == 0) {
            double current_weighted = (incremental_evaluator.getHardScore() < 0) ? 
                evaluator.getWeightedHardConstraintViolations(current_schedule) : 
                incremental_evaluator.getHardScore();
                
            std::cout << "Iteration " << i << ": "
                      << "Best Hard Score = " << best_hard_score
                      << ", Best Soft Score = " << best_soft_score
                      << ", Current Hard Score = " << incremental_evaluator.getHardScore()
                      << " (Weighted: " << current_weighted << ")"
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

    // Show dynamic weights effectiveness summary
    std::cout << "\n=== Dynamic Weights Effectiveness Summary ===" << std::endl;
    std::cout << "Total moves evaluated: " << total_moves_evaluated << std::endl;
    std::cout << "Moves where weighted evaluation made a difference: " << weighted_moves_accepted << std::endl;
    if (total_moves_evaluated > 0) {
        double effectiveness_rate = (double)weighted_moves_accepted / total_moves_evaluated * 100.0;
        std::cout << "Weighted evaluation effectiveness: " << effectiveness_rate << "%" << std::endl;
    }
    
    // Show final weight status
    auto final_weights = evaluator.getDynamicWeights();
    auto final_violations = evaluator.getViolationCounts();
    std::cout << "\nElite solutions quality:" << std::endl;
    for (size_t i = 0; i < elite_solutions.size(); ++i) {
        double hard_score = evaluator.getHardConstraintViolations(elite_solutions[i]);
        double soft_score = evaluator.getSoftConstraintViolations(elite_solutions[i]);
        std::cout << "  Elite " << (i+1) << ": Hard=" << hard_score << ", Soft=" << soft_score;
        if (hard_score == 0) std::cout << " (FEASIBLE!)";
        std::cout << std::endl;
    }
    
    std::cout << "\nFinal constraint weights:" << std::endl;
    for (const auto& pair : final_weights) {
        HardConstraintType constraint_type = pair.first;
        std::string constraint_name;
        switch (constraint_type) {
            case HardConstraintType::WORKING_TIME_CONSTRAINTS: constraint_name = "WorkingTime"; break;
            case HardConstraintType::SHIFT_ROTATION: constraint_name = "ShiftRotation"; break;
            case HardConstraintType::MAX_CONSECUTIVE_SHIFTS: constraint_name = "MaxConsecutive"; break;
            case HardConstraintType::MIN_CONSECUTIVE_SHIFTS: constraint_name = "MinConsecutive"; break;
            case HardConstraintType::MIN_CONSECUTIVE_DAYS_OFF: constraint_name = "MinDaysOff"; break;
            case HardConstraintType::MAX_WEEKENDS_WORKED: constraint_name = "MaxWeekends"; break;
            case HardConstraintType::PRE_ASSIGNED_DAYS_OFF: constraint_name = "PreAssigned"; break;
            case HardConstraintType::MAX_SHIFTS_PER_TYPE: constraint_name = "MaxShiftsPerType"; break;
            case HardConstraintType::MAX_ONE_SHIFT_PER_DAY: constraint_name = "MaxOneShift"; break;
            default: constraint_name = "Unknown"; break;
        }
        
        if (final_violations.at(pair.first) > 0 || pair.second != 1.0) {
            std::cout << "  " << constraint_name << ": weight=" << pair.second 
                      << ", total_violations=" << final_violations.at(pair.first) << std::endl;
        }
    }
    std::cout << "=============================================" << std::endl;

    return best_schedule;
}

Schedule SimulatedAnnealing::generateFeasibleInitialSolution() {
    return initial_solution_generator.generateFeasibleSolution();
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
        // Fallback to feasible initial solution instead of random schedule
        return initial_solution_generator.generateFeasibleSolution();
    }
    
    // Smart selection: prefer feasible solutions, or least infeasible ones
    Schedule best_base = elite_solutions[0];
    double best_hard_score = evaluator.getHardConstraintViolations(best_base);
    
    for (const auto& elite : elite_solutions) {
        double elite_hard_score = evaluator.getHardConstraintViolations(elite);
        
        // Prefer feasible solutions, or less infeasible ones
        if (elite_hard_score > best_hard_score) {
            best_base = elite;
            best_hard_score = elite_hard_score;
        }
    }
    
    std::cout << "  Selected diversification base with hard score: " << best_hard_score << std::endl;
    return best_base;
}

Schedule SimulatedAnnealing::pathRelinkingWithElites() {
    if (elite_solutions.size() < 2) {
        return elite_solutions.empty() ? 
            Schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes()) :
            elite_solutions[0];
    }
    
    // Simple path relinking: combine assignments from two different elite solutions
    Schedule source = elite_solutions[0];
    Schedule target = elite_solutions[elite_solutions.size() - 1];
    Schedule combined = source;
    
    // Copy some assignments from target to source
    for (int emp = 0; emp < instance.getNumEmployees(); ++emp) {
        for (int day = 0; day < instance.getHorizonDays(); ++day) {
            if (Random::getDouble(0.0, 1.0) < 0.3) { // 30% chance to take from target
                combined.setAssignment(emp, day, target.getAssignment(emp, day));
            }
        }
    }
    
    return combined;
}
