#include "soft_constraints.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

SoftConstraints::SoftConstraints(const Instance& inst) : instance(inst) {}

// Helper methods

int SoftConstraints::findEmployeeIndex(const std::string& employee_id) const {
    for (int i = 0; i < instance.getNumEmployees(); i++) {
        if (instance.getStaff(i).ID == employee_id) {
            return i;
        }
    }
    return -1;
}

int SoftConstraints::findShiftIndex(const std::string& shift_id) const {
    for (int i = 0; i < instance.getNumShiftTypes(); i++) {
        if (instance.getShift(i).ShiftID == shift_id) {
            return i + 1; // Return 1-based index for consistency with schedule
        }
    }
    return -1;
}

// Individual soft constraint evaluation methods

int SoftConstraints::evaluateShiftOnRequests(const Schedule& schedule) const {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        score += evaluateShiftOnRequests(schedule, emp);
    }
    return score;
}

int SoftConstraints::evaluateShiftOnRequests(const Schedule& schedule, int employee_id) const {
    int score = 0;
    const auto& on_requests = instance.getShiftOnRequests();
    const Staff& worker = instance.getStaff(employee_id);

    for (const auto& request : on_requests) {
        if (request.EmployeeID == worker.ID) {
            if (request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
                int assigned_shift = schedule.getAssignment(employee_id, request.Day);
                if (assigned_shift != 0) {
                    const Shift& shift_info = instance.getShift(assigned_shift - 1);
                    if (shift_info.ShiftID == request.ShiftID) {
                        score += request.Weight;
                    }
                }
            }
        }
    }
    return score;
}

int SoftConstraints::evaluateShiftOffRequests(const Schedule& schedule) const {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        score += evaluateShiftOffRequests(schedule, emp);
    }
    return score;
}

int SoftConstraints::evaluateShiftOffRequests(const Schedule& schedule, int employee_id) const {
    int score = 0;
    const auto& off_requests = instance.getShiftOffRequests();
    const Staff& worker = instance.getStaff(employee_id);

    for (const auto& request : off_requests) {
        if (request.EmployeeID == worker.ID) {
            if (request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
                int assigned_shift = schedule.getAssignment(employee_id, request.Day);
                if (assigned_shift != 0) {
                    const Shift& shift_info = instance.getShift(assigned_shift - 1);
                    if (shift_info.ShiftID == request.ShiftID) {
                        score += request.Weight; // Note: Weight should be negative for off-requests
                    }
                }
            }
        }
    }
    return score;
}

int SoftConstraints::evaluateCoverageRequirements(const Schedule& schedule) const {
    int score = 0;
    const auto& cover_requirements = instance.getCoverageRequirements();
    
    for (const auto& cover : cover_requirements) {
        if (cover.Day >= 0 && cover.Day < schedule.getHorizonDays()) {
            int shift_index = findShiftIndex(cover.ShiftID);
            
            if (shift_index > 0) {
                int actual_coverage = schedule.getCoverage(cover.Day, shift_index);
                int required_coverage = cover.Requirement;
                
                if (actual_coverage > required_coverage) {
                    // Over-staffing penalty
                    int excess = actual_coverage - required_coverage;
                    score -= excess * abs(cover.Weight_for_over);
                } else if (actual_coverage < required_coverage) {
                    // Under-staffing penalty
                    int deficit = required_coverage - actual_coverage;
                    score -= deficit * abs(cover.Weight_for_under);
                }
                // If actual_coverage == required_coverage, no penalty (score += 0)
            }
        }
    }
    return score;
}

// Aggregate evaluation methods

int SoftConstraints::evaluateAll(const Schedule& schedule) const {
    int total_score = 0;
    
    total_score += evaluateShiftOnRequests(schedule);
    total_score += evaluateShiftOffRequests(schedule);
    total_score += evaluateCoverageRequirements(schedule);
    
    return total_score;
}

int SoftConstraints::evaluateEmployee(const Schedule& schedule, int employee) const {
    if (employee < 0 || employee >= schedule.getNumEmployees()) {
        return 0;
    }
    
    int score = 0;
    score += evaluateShiftOnRequests(schedule, employee);
    score += evaluateShiftOffRequests(schedule, employee);
    
    return score;
}

int SoftConstraints::evaluateMove(const Schedule& schedule, int employee, int day, 
                                  int old_shift, int new_shift) const {
    // Create a temporary schedule with the proposed move
    Schedule temp_schedule = schedule;
    temp_schedule.setAssignment(employee, day, new_shift);
    
    // Calculate the difference in soft constraint score
    int old_score = evaluateAll(schedule);
    int new_score = evaluateAll(temp_schedule);
    
    return new_score - old_score;
}

// Detailed analysis methods

std::map<std::string, int> SoftConstraints::getDetailedScores(const Schedule& schedule) const {
    std::map<std::string, int> scores;
    
    scores["shift_on_requests"] = evaluateShiftOnRequests(schedule);
    scores["shift_off_requests"] = evaluateShiftOffRequests(schedule);
    scores["coverage_requirements"] = evaluateCoverageRequirements(schedule);
    scores["total"] = evaluateAll(schedule);
    
    return scores;
}

std::vector<std::string> SoftConstraints::getUnsatisfiedRequests(const Schedule& schedule) const {
    std::vector<std::string> unsatisfied;
    
    // Check unsatisfied shift-on requests
    const auto& on_requests = instance.getShiftOnRequests();
    for (const auto& request : on_requests) {
        int employee_index = findEmployeeIndex(request.EmployeeID);
        
        if (employee_index >= 0 && request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            bool satisfied = false;
            
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    satisfied = true;
                }
            }
            
            if (!satisfied) {
                std::ostringstream oss;
                oss << "Employee " << request.EmployeeID << " wants " << request.ShiftID 
                    << " on day " << request.Day << " (weight: " << request.Weight << ")";
                unsatisfied.push_back(oss.str());
            }
        }
    }
    
    // Check violated shift-off requests
    const auto& off_requests = instance.getShiftOffRequests();
    for (const auto& request : off_requests) {
        int employee_index = findEmployeeIndex(request.EmployeeID);
        
        if (employee_index >= 0 && request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            bool violated = false;
            
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    violated = true;
                }
            }
            
            if (violated) {
                std::ostringstream oss;
                oss << "Employee " << request.EmployeeID << " assigned unwanted " << request.ShiftID 
                    << " on day " << request.Day << " (penalty: " << request.Weight << ")";
                unsatisfied.push_back(oss.str());
            }
        }
    }
    
    return unsatisfied;
}

std::map<std::string, std::vector<int>> SoftConstraints::getCoverageAnalysis(const Schedule& schedule) const {
    std::map<std::string, std::vector<int>> analysis;
    
    const auto& cover_requirements = instance.getCoverageRequirements();
    
    std::vector<int> required_coverage;
    std::vector<int> actual_coverage;
    std::vector<int> coverage_gaps;
    
    for (const auto& cover : cover_requirements) {
        if (cover.Day >= 0 && cover.Day < schedule.getHorizonDays()) {
            int shift_index = findShiftIndex(cover.ShiftID);
            
            if (shift_index > 0) {
                int actual = schedule.getCoverage(cover.Day, shift_index);
                int required = cover.Requirement;
                int gap = actual - required;
                
                required_coverage.push_back(required);
                actual_coverage.push_back(actual);
                coverage_gaps.push_back(gap);
            }
        }
    }
    
    analysis["required"] = required_coverage;
    analysis["actual"] = actual_coverage;
    analysis["gaps"] = coverage_gaps;
    
    return analysis;
}

// Statistics and reporting

std::map<std::string, double> SoftConstraints::getSatisfactionRates(const Schedule& schedule) const {
    std::map<std::string, double> rates;
    
    // Shift-on request satisfaction rate
    const auto& on_requests = instance.getShiftOnRequests();
    int satisfied_on = 0;
    int total_on = 0;
    
    for (const auto& request : on_requests) {
        int employee_index = findEmployeeIndex(request.EmployeeID);
        
        if (employee_index >= 0 && request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
            total_on++;
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    satisfied_on++;
                }
            }
        }
    }
    
    rates["shift_on_requests"] = total_on > 0 ? (double)satisfied_on / total_on : 1.0;
    
    // Shift-off request satisfaction rate (not violated)
    const auto& off_requests = instance.getShiftOffRequests();
    int violated_off = 0;
    int total_off = 0;
    
    for (const auto& request : off_requests) {
        int employee_index = findEmployeeIndex(request.EmployeeID);
        
        if (employee_index >= 0 && request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
            total_off++;
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    violated_off++;
                }
            }
        }
    }
    
    rates["shift_off_requests"] = total_off > 0 ? (double)(total_off - violated_off) / total_off : 1.0;
    
    // Coverage satisfaction rate
    const auto& cover_requirements = instance.getCoverageRequirements();
    int satisfied_coverage = 0;
    int total_coverage = 0;
    
    for (const auto& cover : cover_requirements) {
        if (cover.Day >= 0 && cover.Day < schedule.getHorizonDays()) {
            int shift_index = findShiftIndex(cover.ShiftID);
            
            if (shift_index > 0) {
                total_coverage++;
                int actual = schedule.getCoverage(cover.Day, shift_index);
                if (actual == cover.Requirement) {
                    satisfied_coverage++;
                }
            }
        }
    }
    
    rates["coverage_requirements"] = total_coverage > 0 ? (double)satisfied_coverage / total_coverage : 1.0;
    
    // Overall satisfaction rate
    double total_rate = (rates["shift_on_requests"] + rates["shift_off_requests"] + rates["coverage_requirements"]) / 3.0;
    rates["overall"] = total_rate;
    
    return rates;
}

int SoftConstraints::getMaxPossibleScore() const {
    int max_score = 0;
    
    // Add all positive weights from shift-on requests
    const auto& on_requests = instance.getShiftOnRequests();
    for (const auto& request : on_requests) {
        if (request.Weight > 0) {
            max_score += request.Weight;
        }
    }
    
    // Note: Shift-off requests and coverage violations typically have negative weights
    // so they don't contribute to the maximum possible score
    
    return max_score;
}

double SoftConstraints::getSatisfactionPercentage(const Schedule& schedule) const {
    int current_score = evaluateAll(schedule);
    int max_score = getMaxPossibleScore();
    
    if (max_score <= 0) {
        return 1.0; // If no positive rewards possible, consider 100% satisfied
    }
    
    // Calculate percentage, ensuring it's between 0 and 1
    double percentage = (double)current_score / max_score;
    return std::max(0.0, std::min(1.0, percentage));
}

// Request analysis

int SoftConstraints::getSatisfiedOnRequests(const Schedule& schedule) const {
    int satisfied = 0;
    const auto& on_requests = instance.getShiftOnRequests();
    
    for (const auto& request : on_requests) {
        int employee_index = findEmployeeIndex(request.EmployeeID);
        
        if (employee_index >= 0 && request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    satisfied++;
                }
            }
        }
    }
    
    return satisfied;
}

int SoftConstraints::getViolatedOffRequests(const Schedule& schedule) const {
    int violated = 0;
    const auto& off_requests = instance.getShiftOffRequests();
    
    for (const auto& request : off_requests) {
        int employee_index = findEmployeeIndex(request.EmployeeID);
        
        if (employee_index >= 0 && request.Day >= 0 && request.Day < schedule.getHorizonDays()) {
            int assigned_shift = schedule.getAssignment(employee_index, request.Day);
            if (assigned_shift != 0) {
                const Shift& shift_info = instance.getShift(assigned_shift - 1);
                if (shift_info.ShiftID == request.ShiftID) {
                    violated++;
                }
            }
        }
    }
    
    return violated;
}

std::map<std::string, int> SoftConstraints::getCoverageGaps(const Schedule& schedule) const {
    std::map<std::string, int> gaps;
    const auto& cover_requirements = instance.getCoverageRequirements();
    
    for (const auto& cover : cover_requirements) {
        if (cover.Day >= 0 && cover.Day < schedule.getHorizonDays()) {
            int shift_index = findShiftIndex(cover.ShiftID);
            
            if (shift_index > 0) {
                int actual = schedule.getCoverage(cover.Day, shift_index);
                int required = cover.Requirement;
                int gap = actual - required;
                
                std::ostringstream key;
                key << "Day" << cover.Day << "_" << cover.ShiftID;
                gaps[key.str()] = gap;
            }
        }
    }
    
    return gaps;
}
