/**
 * NSP Refactored - Main entry point using new data structures
 * 
 * This version uses Instance and Schedule classes instead of raw matrices
 * while maintaining the same Simulated Annealing algorithm
 */

#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include "src/constraints/constraint_evaluator.h"
#include "src/metaheuristics/simulated_annealing.h"
#include "src/utils/random.h"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <ctime>

using namespace std;

string bestSolutionPrint(const Schedule& schedule, const Instance& instance);

int main(int argc, char **argv) {
    Random::initialize();
    if (argc < 6) {
        cerr << "Usage: " << argv[0] << " <instance_file> <iterations> <initial_temp> <cooling_rate> <stagnation_limit>" << endl;
        return 1;
    }
    
    string instance_file = "nsp_instancias/instances1_24/" + string(argv[1]);
    int iterations = stoi(argv[2]);
    double initial_temp = stod(argv[3]);
    double cooling_rate = stod(argv[4]);
    int stagnation_limit = stoi(argv[5]);
    
    cout << "NSP Refactored Version (using Instance and Schedule classes)" << endl;
    cout << "Instance: " << instance_file << endl;
    cout << "Iterations: " << iterations << endl;
    
    // Load instance using the new Instance class
    Instance instance;
    if (!instance.loadFromFile(instance_file)) {
        cerr << "Error: Failed to load instance file" << endl;
        return 1;
    }
    
    cout << "Instance loaded successfully:" << endl;
    cout << "  Employees: " << instance.getNumEmployees() << endl;
    cout << "  Days: " << instance.getHorizonDays() << endl;
    cout << "  Shift types: " << instance.getNumShiftTypes() << endl;
    
    time_t start, end;
    
    // Create unified constraint evaluator
    ConstraintEvaluator evaluator(instance);

    // Create and run Simulated Annealing
    SimulatedAnnealing sa(instance, evaluator, initial_temp, cooling_rate, iterations, stagnation_limit);
    
    cout << "\n=== PHASE 1: Searching for a feasible solution... ===" << endl;
    time(&start);
    Schedule feasible_schedule = sa.solve(SolveMode::Feasibility);
    time(&end);
    
    double hard_score = evaluator.getHardConstraintViolations(feasible_schedule);
    Schedule best_schedule = feasible_schedule;

    if (hard_score < 0) {
        cout << "\nCould not find a feasible solution in Phase 1." << endl;
    } else {
        cout << "\nFeasible solution found! Starting PHASE 2: Optimization." << endl;
        SimulatedAnnealing sa_optimizer(instance, evaluator, initial_temp / 10, cooling_rate, iterations, stagnation_limit);
        
        best_schedule = sa_optimizer.solve(SolveMode::Optimization);
    }
    
    double time_taken = double(end - start);
    
    // Final evaluation
    double best_score = evaluator.getHardConstraintViolations(best_schedule);
    int fitness = evaluator.getSoftConstraintViolations(best_schedule);
    
    cout << "\n=== Final Results ===" << endl;
    cout << "Best score (hard constraints): " << best_score << endl;
    cout << "Fitness (soft constraints): " << fitness << endl;
    
    // Show final constraint analysis
    cout << "\n=== Final Constraint Analysis ===" << endl;
    cout << "Final schedule feasible: " << (evaluator.isFeasible(best_schedule) ? "Yes" : "No") << endl;

    std::map<std::string, int> violations = evaluator.getHardConstraintViolationsMap(best_schedule);
    cout << "\nConstraint Violation Breakdown:" << endl;
    for (const auto& pair : violations) {
        if (pair.second < 0) {
            cout << "  - " << pair.first << ": " << pair.second << endl;
        }
    }
    
    string OutPutLine = bestSolutionPrint(best_schedule, instance);
    
    // Write output file
    OutPutLine.append("Suma de penalizaciones : ");
    OutPutLine.append(to_string(fitness));
    OutPutLine.append("\n");
    
    OutPutLine.append("Factible ? : ");
    if (best_score == 0) {
        OutPutLine.append("Si\n");
    } else {
        OutPutLine.append("No\n");
    }
    
    OutPutLine.append("Tiempo total de ejecucion: ");
    OutPutLine.append(to_string(time_taken));
    OutPutLine.append("[s]\n");
    
    string outFile = "./instancias_solucion/";
    outFile.append(argv[1]);
    ofstream outdata(outFile);
    outdata << OutPutLine;
    
    return 0;
}

// All constraint evaluation is now handled by the unified ConstraintEvaluator class

string bestSolutionPrint(const Schedule& schedule, const Instance& instance) {
    string line;
    for (int i = 0; i < schedule.getNumEmployees(); i++) {
        const Staff& staff = instance.getStaff(i);
        line.append(staff.ID);
        line.append(":");
        for (int j = 0; j < schedule.getHorizonDays(); j++) {
            int shift = schedule.getAssignment(i, j);
            if (shift != 0) {
                const Shift& shift_info = instance.getShift(shift - 1);
                line.append(" (");
                line.append(to_string(j));
                line.append(",");
                line.append(shift_info.ShiftID);
                line.append(")");
            }
        }
        line.append("\n");
    }
    return line;
}
