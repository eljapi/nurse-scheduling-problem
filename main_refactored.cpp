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
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <ctime>

using namespace std;
double Acceptance(double score_actual, double score_final, double T, double EulerConstant);
string bestSolutionPrint(const Schedule& schedule, const Instance& instance);

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <instance_file> <iterations>" << endl;
        return 1;
    }
    
    string instance_file = "nsp_instancias/instances1_24/" + string(argv[1]);
    int iterations = stoi(argv[2]);
    
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
    
    // Initialize algorithm parameters
    int Cantidad_empleados = instance.getNumEmployees();
    int SECTION_HORIZON = instance.getHorizonDays();
    int turnos_max = instance.getNumShiftTypes();
    
    // Create schedules using the new Schedule class
    Schedule current_schedule(Cantidad_empleados, SECTION_HORIZON);
    Schedule aux_schedule(Cantidad_empleados, SECTION_HORIZON);
    Schedule best_schedule(Cantidad_empleados, SECTION_HORIZON);
    
    // Initialize the schedule randomly
    current_schedule.randomize(turnos_max);
    aux_schedule.copyFrom(current_schedule);
    
    // Algorithm parameters
    double score = 0;
    double current_score = -1 * pow(10, 5);
    double best_score = -1 * pow(10, 5);
    double T = 0;
    double Reset_T = 0;
    double EulerConstant = 2.71828;
    int estancado = 0;
    int iter_estancado = iterations / 5;
    int Reset_max = 0;
    int index = 0;
    
    time_t start, end;
    
    // Create unified constraint evaluator
    ConstraintEvaluator evaluator(instance);
    
    // Calculate initial score using unified evaluator
    score = evaluator.getHardConstraintViolations(current_schedule);
    T = abs(score) * 10;
    Reset_T = T;
    current_score = score;
    best_score = score;
    
    cout << "Initial score: " << score << endl;
    cout << "Is initial schedule feasible: " << (evaluator.isFeasible(current_schedule) ? "Yes" : "No") << endl;
    
    // Main optimization loop
    time(&start);
    while (index < iterations) {
        bool flag = true;
        
        for (int i = 0; i < Cantidad_empleados; i++) {
            if (flag) {
                for (int j = 0; j < SECTION_HORIZON; j++) {
                    int turno = aux_schedule.getAssignment(i, j);
                    if (flag) {
                        for (int w = 0; w <= turnos_max; w++) {
                            if (turno != w) {
                                aux_schedule.setAssignment(i, j, w);
                            }
                            
                            // Calculate new score using unified evaluator
                            score = evaluator.getHardConstraintViolations(aux_schedule);
                            
                            // Simulated Annealing acceptance
                            unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
                            default_random_engine e(seed);
                            uniform_real_distribution<double> distR(0, 1);
                            double p = distR(e);
                            
                            double Accept = Acceptance(score, current_score, T, EulerConstant);
                            
                            cout << "Score actual: " << score << " Mejor Score: " << best_score 
                                 << " Estancado: " << estancado << endl;
                            
                            if (Accept > p) {
                                current_score = score;
                                current_schedule.copyFrom(aux_schedule);
                                flag = false;

                                if (current_score > best_score) {
                                    estancado = 0;
                                    best_score = current_score;
                                    best_schedule.copyFrom(current_schedule);
                                    cout << "New best score: " << best_score << endl;
                                }
                                
                                // Check for optimal solution (feasible)
                                if (best_score == 0) {
                                    cout << "Optimal solution found!" << endl;
                                    index = iterations; // Force exit from main loop
                                }
                                break;
                            } else {
                                aux_schedule.setAssignment(i, j, turno);
                            }
                        }
                    } else {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        if (best_score == 0) {
            break;
        }
        
        if (current_score <= best_score) {
            estancado += 1;
        }
        
        if (estancado > iter_estancado) {
            cout << estancado << " estancado" << endl;
            Reset_max += 1;
            
            T = Reset_T;
            estancado = 0;
            
            current_schedule.randomize(turnos_max);
            aux_schedule.copyFrom(current_schedule);
            current_score = evaluator.getHardConstraintViolations(current_schedule);
        }
        
        T = T * (1 - (index + 1.0) / iterations);
        index++;
    }
    
    time(&end);
    double time_taken = double(end - start);
    
    // Final evaluation
    cout << setprecision(5);
    cout << current_score << endl;
    cout << best_score << endl;
    
    // Print final schedule
    for (int i = 0; i < Cantidad_empleados; i++) {
        for (int j = 0; j < SECTION_HORIZON; j++) {
            if (j + 1 == SECTION_HORIZON) {
                cout << best_schedule.getAssignment(i, j) << " " << endl;
            } else {
                cout << best_schedule.getAssignment(i, j) << " ";
            }
        }
    }
    
    // Final score calculation using unified evaluator
    score = evaluator.getHardConstraintViolations(best_schedule);
    
    int fitness = evaluator.getSoftConstraintPenalties(best_schedule);
    
    cout << score << endl;
    cout << fitness << " Fitness" << endl;
    
    // Show final constraint analysis
    cout << "\n=== Final Constraint Analysis ===" << endl;
    cout << "Final schedule feasible: " << (evaluator.isFeasible(best_schedule) ? "Yes" : "No") << endl;
    
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

double Acceptance(double score_actual, double score_final, double T, double EulerConstant) {
    if (score_actual > score_final) {
        return 1.0;
    }
    if (T == 0) return 0;
    return exp((score_actual - score_final) / T);
}

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
