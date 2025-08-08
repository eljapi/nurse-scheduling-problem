#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "../core/instance.h"
#include "../core/data_structures.h"
#include "../constraints/incremental_evaluator.h"
#include "../core/move.h"
#include "neighborhood.h"
#include "initial_solution.h"
#include <unordered_set>
#include <deque>
#include <vector>

enum class SolveMode { Feasibility, Optimization };

// Tabu-like memory structure to avoid cycling
struct TabuMemory {
    std::deque<std::pair<int, int>> recent_moves; // (employee, day) pairs
    std::unordered_set<std::string> tabu_assignments; // String representation of assignments
    int max_size;
    
    TabuMemory(int size) : max_size(size) {}
    
    void addMove(int employee, int day, int shift);
    bool isTabu(int employee, int day, int shift) const;
    void clear();
    std::string getAssignmentKey(int employee, int day, int shift) const;
};

// Diversification and intensification strategies
class DiversificationIntensification {
public:
    DiversificationIntensification(const Instance& instance, ConstraintEvaluator& evaluator);
    
    // Diversification strategies
    Schedule diversifyRestart(const Schedule& current_best, double perturbation_rate);
    Schedule diversifyRandomRestart(const Schedule& current_best);
    Schedule diversifyGuidedRestart(const Schedule& current_best);
    
    // Intensification strategies
    Schedule intensifyLocalSearch(const Schedule& schedule, int max_iterations);
    Schedule intensifyHillClimbing(const Schedule& schedule, int max_iterations);
    Schedule intensifyVariableNeighborhood(const Schedule& schedule, int max_iterations);
    
private:
    const Instance& instance;
    ConstraintEvaluator& evaluator;
    Neighborhood neighborhood;
};

class SimulatedAnnealing {
public:
    SimulatedAnnealing(const Instance& instance, ConstraintEvaluator& evaluator,
                       double initial_temp, double cooling, int max_iter, int stagnation,
                       int weight_update_freq = -1); // Default: stagnation_limit / 2

    Schedule solve(SolveMode mode);
    Schedule solve(const Schedule& initial_schedule, SolveMode mode);
    
    // Generate feasible initial solution using the 5-step heuristic
    Schedule generateFeasibleInitialSolution();

private:
    const Instance& instance;
    ConstraintEvaluator& evaluator;
    IncrementalEvaluator incremental_evaluator;
    Neighborhood neighborhood;
    TabuMemory tabu_memory;
    DiversificationIntensification div_int_strategies;
    InitialSolutionGenerator initial_solution_generator;

    // SA Parameters
    double initial_temperature;
    double cooling_rate;
    int max_iterations;
    int stagnation_limit;
    int weight_update_frequency;
    
    // Diversification/Intensification parameters
    int restart_count;
    int max_restarts;
    int intensification_frequency;
    int diversification_frequency;
    std::vector<Schedule> elite_solutions;
    int elite_size;
    
    // Statistics for dynamic weights effectiveness
    int weighted_moves_accepted;
    int total_moves_evaluated;

    double acceptance(double delta, double temperature);
    bool shouldDiversify(int iterations_since_improvement);
    bool shouldIntensify(int iterations_since_improvement);
    void updateEliteSolutions(const Schedule& schedule, double hard_score, double soft_score);
    Schedule selectDiversificationBase();
    Schedule pathRelinkingWithElites();
};

#endif // SIMULATED_ANNEALING_H
