/**
 * NSP Refactored - Main entry point using new data structures
 * 
 * This version uses Instance and Schedule classes instead of raw matrices
 * while maintaining the same Simulated Annealing algorithm
 */

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
    double score_final = -1 * pow(10, 5);
    double score_pasado = 0;
    double Maximo = -1 * pow(10, 5);
    double T = 100;
    double Reset_T = 100;
    double EulerConstant = 2.71828;
    int estancado = 0;
    int iter_estancado = 15;
    int Reset_max = 0;
    int index = 0;
    
    time_t start, end;
    
    // Create unified constraint evaluator
    ConstraintEvaluator evaluator(instance);
    
    // Calculate initial score using unified evaluator
    score = evaluator.evaluateHardConstraints(current_schedule);
    
    cout << "Initial score: " << score << endl;
    cout << "Is initial schedule feasible: " << (evaluator.isFeasible(current_schedule) ? "Yes" : "No") << endl;
    
    // Show constraint violations
    auto violations = evaluator.getViolationReport(current_schedule);
    if (!violations.empty()) {
        cout << "Initial violations:" << endl;
        for (const auto& violation : violations) {
            cout << "  - " << violation << endl;
        }
    }
    
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
                            score = evaluator.evaluateHardConstraints(aux_schedule);
                            
                            // Simulated Annealing acceptance
                            unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
                            default_random_engine e(seed);
                            uniform_real_distribution<double> distR(0, 1);
                            double p = distR(e);
                            
                            double Accept = Acceptance(score, score_final, T, EulerConstant);
                            
                            cout << "Score actual: " << score << " Mejor Score: " << score_final 
                                 << " Estancado: " << estancado << endl;
                            score_pasado = score;
                            
                            if (score > score_final) {
                                estancado = 0;
                                score_final = score;
                                flag = false; // Some improvement, change neighborhood
                                current_schedule.copyFrom(aux_schedule);
                                
                                cout << "Score total: " << score_final << endl;
                                
                                // Check for optimal solution (feasible)
                                if (score_final == 0) {
                                    best_schedule.copyFrom(current_schedule);
                                    Maximo = score_final;
                                    cout << "Optimal solution found!" << endl;
                                    index = iterations; // Force exit from main loop
                                }
                                break;
                            } else if (p < Accept && Accept != -1) {
                                flag = false;
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
        
        if (score_final == 0) {
            best_schedule.copyFrom(current_schedule);
            Maximo = score_final;
            break;
        }
        
        if (score <= score_final) {
            estancado += 1;
        }
        
        if (estancado > iter_estancado) {
            cout << estancado << " estancado" << endl;
            Reset_max += 1;
            cout << score_final << endl;
            
            T = Reset_T;
            estancado = 0;
            if (Maximo < score_final) {
                Maximo = score_final;
                best_schedule.copyFrom(current_schedule);
            }
            
            score_final = -1 * pow(10, 5);
            aux_schedule.randomize(turnos_max);
        }
        
        T = T * (1 - (index + 1.0) / iterations);
        index++;
    }
    
    time(&end);
    double time_taken = double(end - start);
    
    // Final evaluation
    cout << setprecision(5);
    cout << score_final << endl;
    cout << Maximo << endl;
    
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
    score = evaluator.evaluateHardConstraints(best_schedule);
    
    int fitness = evaluator.evaluateSoftConstraints(best_schedule);
    
    cout << score << endl;
    cout << fitness << " Fitness" << endl;
    
    // Show final constraint analysis
    cout << "\n=== Final Constraint Analysis ===" << endl;
    cout << "Final schedule feasible: " << (evaluator.isFeasible(best_schedule) ? "Yes" : "No") << endl;
    
    auto final_violations = evaluator.getViolationReport(best_schedule);
    cout << "Final violations: " << final_violations.size() << endl;
    for (const auto& violation : final_violations) {
        cout << "  - " << violation << endl;
    }
    
    // Show constraint statistics
    auto stats = evaluator.getConstraintStatistics(best_schedule);
    cout << "\nConstraint satisfaction rates:" << endl;
    for (const auto& stat : stats) {
        cout << "  " << stat.first << ": " << (stat.second * 100) << "%" << endl;
    }
    
    string OutPutLine = bestSolutionPrint(best_schedule, instance);
    
    // Write output file
    OutPutLine.append("Suma de penalizaciones : ");
    OutPutLine.append(to_string(fitness));
    OutPutLine.append("\n");
    
    OutPutLine.append("Factible ? : ");
    if (Maximo == 0) {
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
    } else {
        double delta = abs(score_actual - score_final);
        return exp(-delta / T);
    }
}
        
        // Count shifts per worker
        for (int shift_type = 1; shift_type <= num_shifts; shift_type++) {
            int shift_count = schedule.getShiftCount(i, shift_type);
            
            // Check max shifts constraint
            if (shift_type - 1 < static_cast<int>(worker.MaxShifts.size())) {
                int max_shifts = stoi(worker.MaxShifts[shift_type - 1]);
                if (shift_count > max_shifts) {
                    score -= 10;
                }
            }
        }
    }
    return score;
}

int ShiftTimesSum(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        int total_minutes = 0;
        
        // Calculate total minutes worked
        for (int shift_type = 1; shift_type <= instance.getNumShiftTypes(); shift_type++) {
            int shift_count = schedule.getShiftCount(i, shift_type);
            const Shift& shift_info = instance.getShift(shift_type - 1);
            total_minutes += shift_count * shift_info.mins;
        }
        
        // Check min/max total minutes constraints
        if (total_minutes > worker.MaxTotalMinutes) {
            score -= 10;
        }
        if (total_minutes < worker.MinTotalMinutes) {
            score -= 10;
        }
    }
    return score;
}

int maxConsecutiveShifts(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        int consecutive_count = 0;
        
        for (int j = 0; j < horizon; j++) {
            if (schedule.getAssignment(i, j) != 0) {
                consecutive_count++;
                if (consecutive_count > worker.MaxConsecutiveShifts) {
                    score -= 10;
                }
            } else {
                consecutive_count = 0;
            }
        }
    }
    return score;
}

int minConsecutiveShifts(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        int consecutive_days_off = 0;
        
        for (int j = 0; j < horizon; j++) {
            if (schedule.getAssignment(i, j) == 0) {
                consecutive_days_off++;
                if (consecutive_days_off > worker.MinConsecutiveShifts) {
                    score -= 80;
                }
            } else {
                consecutive_days_off = 0;
            }
        }
    }
    return score;
}

int MaxConsecutiveWeekendWork(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        int weekend_count = 0;
        
        // Check weekends (assuming Saturday=5, Sunday=6 in 0-based indexing)
        for (int weekend_start = 5; weekend_start < horizon; weekend_start += 7) {
            if (weekend_start + 1 < horizon) {
                if (schedule.getAssignment(i, weekend_start) != 0 || 
                    schedule.getAssignment(i, weekend_start + 1) != 0) {
                    weekend_count++;
                }
            }
        }
        
        if (weekend_count > worker.MaxWeekends) {
            score -= 100 * weekend_count;
        }
    }
    return score;
}

int MustDayoff(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    const auto& days_off_list = instance.getDaysOff();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        
        // Find days off for this employee
        for (const auto& days_off : days_off_list) {
            if (days_off.EmployeeID == worker.ID) {
                for (const auto& day_str : days_off.DayIndexes) {
                    int day_index = stoi(day_str);
                    if (day_index >= 0 && day_index < horizon) {
                        if (schedule.getAssignment(i, day_index) != 0) {
                            score -= 1000;
                        }
                    }
                }
                break;
            }
        }
    }
    return score;
}

int CantFollowRestriction(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon - 1; j++) {
            int current_shift = schedule.getAssignment(i, j);
            int next_shift = schedule.getAssignment(i, j + 1);
            
            if (current_shift != 0 && next_shift != 0) {
                const Shift& current_shift_info = instance.getShift(current_shift - 1);
                const Shift& next_shift_info = instance.getShift(next_shift - 1);
                
                // Check if next shift is in the "can't follow" list
                for (const auto& cant_follow : current_shift_info.cant_follow) {
                    if (cant_follow.compare("\n") != 3) {
                        if (next_shift_info.ShiftID[0] == cant_follow[0]) {
                            score -= 100;
                        }
                    }
                }
            }
        }
    }
    return score;
}

int ShiftOnRequest(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    const auto& on_requests = instance.getShiftOnRequests();
    
    for (const auto& request : on_requests) {
        // Find employee index by ID
        int employee_index = -1;
        for (int i = 0; i < instance.getNumEmployees(); i++) {
            if (instance.getStaff(i).ID == request.EmployeeID) {
                employee_index = i;
                break;
            }
        }
        
        if (employee_index >= 0) {
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    score += request.Weight;
                }
            }
        }
    }
    return score;
}

int ShiftOffRequest(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    const auto& off_requests = instance.getShiftOffRequests();
    
    for (const auto& request : off_requests) {
        // Find employee index by ID
        int employee_index = -1;
        for (int i = 0; i < instance.getNumEmployees(); i++) {
            if (instance.getStaff(i).ID == request.EmployeeID) {
                employee_index = i;
                break;
            }
        }
        
        if (employee_index >= 0) {
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    score += request.Weight;
                }
            }
        }
    }
    return score;
}

int SectionCover(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    const auto& cover_requirements = instance.getCoverageRequirements();
    
    for (const auto& cover : cover_requirements) {
        int day = cover.Day;
        string shift_id = cover.ShiftID;
        int requirement = cover.Requirement;
        
        // Find shift index by ID
        int shift_index = -1;
        for (int i = 0; i < instance.getNumShiftTypes(); i++) {
            if (instance.getShift(i).ShiftID == shift_id) {
                shift_index = i + 1; // 1-based indexing
                break;
            }
        }
        
        if (shift_index > 0) {
            int coverage = schedule.getCoverage(day, shift_index);
            
            if (coverage > requirement) {
                score += (coverage - requirement) * cover.Weight_for_over;
            } else if (coverage < requirement) {
                score += (requirement - coverage) * cover.Weight_for_under;
            }
        }
    }
    return score;
}

double Acceptance(double score_actual, double score_final, double T, double EulerConstant) {
    if (score_actual > score_final) {
        return 1.0;
    } else {
        double delta = abs(score_actual - score_final);
        return exp(-delta / T);
    }
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
                line.append(shift_info.ShiftID);
            } else {
                line.append("-");
            }
            if (j + 1 != schedule.getHorizonDays()) {
                line.append(" ");
            }
        }
        line.append("\n");
    }
    return line;
}