#include "hard_constraints.h"
#include <algorithm>
#include <sstream>
#include <map>
#include <vector>

HardConstraints::HardConstraints(const Instance& inst) : instance(inst) {}

// Helper methods

bool HardConstraints::isValidShiftTransition(int current_shift, int next_shift) const {
    if (current_shift == 0 || next_shift == 0) {
        return true; // No restriction if either is a day off
    }
    
    const Shift& current_shift_info = instance.getShift(current_shift - 1);
    
    // Check if next shift is in the "can't follow" list
    for (const auto& cant_follow : current_shift_info.cant_follow) {
        if (cant_follow.compare("\n") != 3) {
            const Shift& next_shift_info = instance.getShift(next_shift - 1);
            if (next_shift_info.ShiftID[0] == cant_follow[0]) {
                return false;
            }
        }
    }
    return true;
}

int HardConstraints::countConsecutiveWork(const Schedule& schedule, int employee, int start_day) const {
    int count = 0;
    int horizon = schedule.getHorizonDays();
    
    for (int day = start_day; day < horizon; day++) {
        if (schedule.getAssignment(employee, day) != 0) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

int HardConstraints::countConsecutiveDaysOff(const Schedule& schedule, int employee, int start_day) const {
    int count = 0;
    int horizon = schedule.getHorizonDays();
    
    for (int day = start_day; day < horizon; day++) {
        if (schedule.getAssignment(employee, day) == 0) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

int HardConstraints::countWeekendsWorked(const Schedule& schedule, int employee) const {
    int weekend_count = 0;
    int horizon = schedule.getHorizonDays();
    
    // Check weekends (assuming Saturday=5, Sunday=6 in 0-based indexing)
    for (int weekend_start = 5; weekend_start < horizon; weekend_start += 7) {
        if (weekend_start + 1 < horizon) {
            if (schedule.getAssignment(employee, weekend_start) != 0 || 
                schedule.getAssignment(employee, weekend_start + 1) != 0) {
                weekend_count++;
            }
        }
    }
    return weekend_count;
}

// Individual constraint evaluation methods

int HardConstraints::evaluateMaxOneShiftPerDay(const Schedule& schedule) const {
    // This constraint is implicitly satisfied by our Schedule class design
    // since setAssignment only allows one shift per employee per day
    return 0;
}

int HardConstraints::evaluateShiftRotation(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateShiftRotation(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateShiftRotation(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    int horizon = schedule.getHorizonDays();
    for (int day = 0; day < horizon - 1; day++) {
        int current_shift = schedule.getAssignment(employee_id, day);
        int next_shift = schedule.getAssignment(employee_id, day + 1);
        
        if (!isValidShiftTransition(current_shift, next_shift)) {
            penalty -= 100; // High penalty for rotation violations
        }
    }
    return penalty;
}

int HardConstraints::evaluateMaxShiftsPerType(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateMaxShiftsPerType(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateMaxShiftsPerType(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    int num_shifts = instance.getNumShiftTypes();
    const Staff& worker = instance.getStaff(employee_id);
    
    for (int shift_type = 1; shift_type <= num_shifts; shift_type++) {
        int shift_count = schedule.getShiftCount(employee_id, shift_type);
        
        // Check max shifts constraint
        if (shift_type - 1 < static_cast<int>(worker.MaxShifts.size())) {
            if (worker.MaxShifts[shift_type - 1] != "None") {
                int max_shifts = std::stoi(worker.MaxShifts[shift_type - 1]);
                if (shift_count > max_shifts) {
                    penalty -= 10 * (shift_count - max_shifts);
                }
            }
        }
    }
    return penalty;
}

int HardConstraints::evaluateWorkingTimeConstraints(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateWorkingTimeConstraints(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateWorkingTimeConstraints(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    const Staff& worker = instance.getStaff(employee_id);
    int total_minutes = 0;
    
    // Calculate total minutes worked
    for (int shift_type = 1; shift_type <= instance.getNumShiftTypes(); shift_type++) {
        int shift_count = schedule.getShiftCount(employee_id, shift_type);
        const Shift& shift_info = instance.getShift(shift_type - 1);
        total_minutes += shift_count * shift_info.mins;
    }
    
    // Check min/max total minutes constraints
    if (total_minutes > worker.MaxTotalMinutes) {
        penalty -= 10;
    }
    if (total_minutes < worker.MinTotalMinutes) {
        penalty -= 10;
    }
    return penalty;
}

int HardConstraints::evaluateMaxConsecutiveShifts(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateMaxConsecutiveShifts(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateMaxConsecutiveShifts(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    int horizon = schedule.getHorizonDays();
    const Staff& worker = instance.getStaff(employee_id);
    int consecutive_count = 0;
    
    for (int day = 0; day < horizon; day++) {
        if (schedule.getAssignment(employee_id, day) != 0) {
            consecutive_count++;
            if (consecutive_count > worker.MaxConsecutiveShifts) {
                penalty -= 10;
            }
        } else {
            consecutive_count = 0;
        }
    }
    return penalty;
}

int HardConstraints::evaluateMinConsecutiveShifts(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateMinConsecutiveShifts(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateMinConsecutiveShifts(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    int horizon = schedule.getHorizonDays();
    const Staff& worker = instance.getStaff(employee_id);
    bool in_work_period = false;
    int work_period_length = 0;
    
    for (int day = 0; day < horizon; day++) {
        if (schedule.getAssignment(employee_id, day) != 0) {
            if (!in_work_period) {
                in_work_period = true;
            }
            work_period_length++;
        } else {
            if (in_work_period) {
                // End of work period, check if it meets minimum
                if (work_period_length < worker.MinConsecutiveShifts) {
                    penalty -= 50;
                }
                in_work_period = false;
            }
            work_period_length = 0;
        }
    }
    
    // Check final work period
    if (in_work_period && work_period_length < worker.MinConsecutiveShifts) {
        penalty -= 50;
    }
    return penalty;
}

int HardConstraints::evaluateMinConsecutiveDaysOff(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateMinConsecutiveDaysOff(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateMinConsecutiveDaysOff(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    int horizon = schedule.getHorizonDays();
    const Staff& worker = instance.getStaff(employee_id);
    int consecutive_off_count = 0;
    bool in_off_period = false;

    for (int day = 0; day < horizon; ++day) {
        if (schedule.getAssignment(employee_id, day) == 0) {
            if (!in_off_period) {
                in_off_period = true;
            }
            consecutive_off_count++;
        } else {
            if (in_off_period) {
                if (consecutive_off_count < worker.MinConsecutiveDaysOff) {
                    penalty -= 60;
                }
                in_off_period = false;
            }
            consecutive_off_count = 0;
        }
    }
    if (in_off_period && consecutive_off_count < worker.MinConsecutiveDaysOff) {
        penalty -= 60;
    }
    return penalty;
}

int HardConstraints::evaluateMaxWeekendsWorked(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluateMaxWeekendsWorked(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluateMaxWeekendsWorked(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    const Staff& worker = instance.getStaff(employee_id);
    int weekend_count = countWeekendsWorked(schedule, employee_id);
    
    if (weekend_count > worker.MaxWeekends) {
        penalty -= 100 * weekend_count;
    }
    return penalty;
}

int HardConstraints::evaluatePreAssignedDaysOff(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    for (int emp = 0; emp < num_employees; emp++) {
        penalty += evaluatePreAssignedDaysOff(schedule, emp);
    }
    return penalty;
}

int HardConstraints::evaluatePreAssignedDaysOff(const Schedule& schedule, int employee_id) const {
    int penalty = 0;
    int horizon = schedule.getHorizonDays();
    const auto& days_off_list = instance.getDaysOff();
    const Staff& worker = instance.getStaff(employee_id);
    
    // Find days off for this employee
    for (const auto& days_off : days_off_list) {
        if (days_off.EmployeeID == worker.ID) {
            for (const auto& day_str : days_off.DayIndexes) {
                int day_index = std::stoi(day_str);
                if (day_index >= 0 && day_index < horizon) {
                    if (schedule.getAssignment(employee_id, day_index) != 0) {
                        penalty -= 1000; // Very high penalty for violating mandatory days off
                    }
                }
            }
            break;
        }
    }
    return penalty;
}

// Aggregate evaluation methods

int HardConstraints::evaluateAll(const Schedule& schedule) const {
    int total_penalty = 0;
    
    total_penalty += evaluateMaxShiftsPerType(schedule);
    total_penalty += evaluateWorkingTimeConstraints(schedule);
    total_penalty += evaluateMaxConsecutiveShifts(schedule);
    total_penalty += evaluateMinConsecutiveShifts(schedule);
    total_penalty += evaluateMinConsecutiveDaysOff(schedule);
    total_penalty += evaluateMaxWeekendsWorked(schedule);
    total_penalty += evaluatePreAssignedDaysOff(schedule);
    total_penalty += evaluateShiftRotation(schedule);
    
    return total_penalty;
}

bool HardConstraints::isFeasible(const Schedule& schedule) const {
    return evaluateAll(schedule) == 0;
}

int HardConstraints::evaluateEmployee(const Schedule& schedule, int employee) const {
    int total_penalty = 0;
    total_penalty += evaluateShiftRotation(schedule, employee);
    total_penalty += evaluateMaxShiftsPerType(schedule, employee);
    total_penalty += evaluateWorkingTimeConstraints(schedule, employee);
    total_penalty += evaluateMaxConsecutiveShifts(schedule, employee);
    total_penalty += evaluateMinConsecutiveShifts(schedule, employee);
    total_penalty += evaluateMinConsecutiveDaysOff(schedule, employee);
    total_penalty += evaluateMaxWeekendsWorked(schedule, employee);
    total_penalty += evaluatePreAssignedDaysOff(schedule, employee);
    return total_penalty;
}

int HardConstraints::evaluateMove(const Schedule& schedule, int employee, int day, 
                                  int old_shift, int new_shift) const {
    // Create a copy of the schedule with the proposed move
    Schedule temp_schedule = schedule;
    temp_schedule.setAssignment(employee, day, new_shift);
    
    // Calculate the difference in penalty
    int old_penalty = evaluateAll(schedule);
    int new_penalty = evaluateAll(temp_schedule);
    
    return new_penalty - old_penalty;
}

// Detailed constraint information

std::vector<std::string> HardConstraints::getViolationDetails(const Schedule& schedule) const {
    std::vector<std::string> violations;
    
    if (evaluateShiftRotation(schedule) < 0) {
        violations.push_back("Shift rotation violations detected");
    }
    if (evaluateMaxShiftsPerType(schedule) < 0) {
        violations.push_back("Maximum shifts per type exceeded");
    }
    if (evaluateWorkingTimeConstraints(schedule) < 0) {
        violations.push_back("Working time constraints violated");
    }
    if (evaluateMaxConsecutiveShifts(schedule) < 0) {
        violations.push_back("Maximum consecutive shifts exceeded");
    }
    if (evaluateMinConsecutiveShifts(schedule) < 0) {
        violations.push_back("Minimum consecutive shifts not met");
    }
    if (evaluateMinConsecutiveDaysOff(schedule) < 0) {
        violations.push_back("Minimum consecutive days off not met");
    }
    if (evaluateMaxWeekendsWorked(schedule) < 0) {
        violations.push_back("Maximum weekends worked exceeded");
    }
    if (evaluatePreAssignedDaysOff(schedule) < 0) {
        violations.push_back("Pre-assigned days off violated");
    }
    
    return violations;
}

std::map<std::string, int> HardConstraints::getPenaltyWeights() const {
    std::map<std::string, int> weights;
    weights["shift_rotation"] = 100;
    weights["max_shifts_per_type"] = 10;
    weights["working_time"] = 10;
    weights["max_consecutive_shifts"] = 10;
    weights["min_consecutive_shifts"] = 80;
    weights["min_consecutive_days_off"] = 50;
    weights["max_weekends"] = 100;
    weights["pre_assigned_days_off"] = 1000;
    return weights;
}

std::map<std::string, double> HardConstraints::getConstraintStatistics(const Schedule& schedule) const {
    std::map<std::string, double> stats;
    
    // Calculate satisfaction rates for each constraint type
    stats["shift_rotation"] = (evaluateShiftRotation(schedule) == 0) ? 1.0 : 0.0;
    stats["max_shifts_per_type"] = (evaluateMaxShiftsPerType(schedule) == 0) ? 1.0 : 0.0;
    stats["working_time"] = (evaluateWorkingTimeConstraints(schedule) == 0) ? 1.0 : 0.0;
    stats["max_consecutive_shifts"] = (evaluateMaxConsecutiveShifts(schedule) == 0) ? 1.0 : 0.0;
    stats["min_consecutive_shifts"] = (evaluateMinConsecutiveShifts(schedule) == 0) ? 1.0 : 0.0;
    stats["min_consecutive_days_off"] = (evaluateMinConsecutiveDaysOff(schedule) == 0) ? 1.0 : 0.0;
    stats["max_weekends"] = (evaluateMaxWeekendsWorked(schedule) == 0) ? 1.0 : 0.0;
    stats["pre_assigned_days_off"] = (evaluatePreAssignedDaysOff(schedule) == 0) ? 1.0 : 0.0;
    
    // Overall feasibility rate
    double total_satisfied = 0;
    for (const auto& pair : stats) {
        total_satisfied += pair.second;
    }
    stats["overall_feasibility"] = total_satisfied / stats.size();
    
    return stats;
}

std::vector<std::pair<int, int>> HardConstraints::getViolatingAssignments(const Schedule& schedule) const {
    std::vector<std::pair<int, int>> violating_assignments;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();

    for (int emp = 0; emp < num_employees; ++emp) {
        if (evaluateMaxShiftsPerType(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluateWorkingTimeConstraints(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluateMaxConsecutiveShifts(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluateMinConsecutiveShifts(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluateMinConsecutiveDaysOff(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluateMaxWeekendsWorked(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluatePreAssignedDaysOff(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
        if (evaluateShiftRotation(schedule, emp) < 0) {
            for (int day = 0; day < horizon; ++day) {
                violating_assignments.push_back({emp, day});
            }
        }
    }
    return violating_assignments;
}

std::map<std::string, int> HardConstraints::getConstraintViolations(const Schedule& schedule) const {
    std::map<std::string, int> violations;
    violations["MaxShiftsPerType"] = evaluateMaxShiftsPerType(schedule);
    violations["WorkingTime"] = evaluateWorkingTimeConstraints(schedule);
    violations["MaxConsecutiveShifts"] = evaluateMaxConsecutiveShifts(schedule);
    violations["MinConsecutiveShifts"] = evaluateMinConsecutiveShifts(schedule);
    violations["MinConsecutiveDaysOff"] = evaluateMinConsecutiveDaysOff(schedule);
    violations["MaxWeekendsWorked"] = evaluateMaxWeekendsWorked(schedule);
    violations["PreAssignedDaysOff"] = evaluatePreAssignedDaysOff(schedule);
    violations["ShiftRotation"] = evaluateShiftRotation(schedule);
    return violations;
}
