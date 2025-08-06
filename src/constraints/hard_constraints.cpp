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
    int horizon = schedule.getHorizonDays();
    
    for (int emp = 0; emp < num_employees; emp++) {
        for (int day = 0; day < horizon - 1; day++) {
            int current_shift = schedule.getAssignment(emp, day);
            int next_shift = schedule.getAssignment(emp, day + 1);
            
            if (!isValidShiftTransition(current_shift, next_shift)) {
                penalty -= 100; // High penalty for rotation violations
            }
        }
    }
    return penalty;
}

int HardConstraints::evaluateMaxShiftsPerType(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    int num_shifts = instance.getNumShiftTypes();
    
    for (int emp = 0; emp < num_employees; emp++) {
        const Staff& worker = instance.getStaff(emp);
        
        for (int shift_type = 1; shift_type <= num_shifts; shift_type++) {
            int shift_count = schedule.getShiftCount(emp, shift_type);
            
            // Check max shifts constraint
            if (shift_type - 1 < static_cast<int>(worker.MaxShifts.size())) {
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
        const Staff& worker = instance.getStaff(emp);
        int total_minutes = 0;
        
        // Calculate total minutes worked
        for (int shift_type = 1; shift_type <= instance.getNumShiftTypes(); shift_type++) {
            int shift_count = schedule.getShiftCount(emp, shift_type);
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
    }
    return penalty;
}

int HardConstraints::evaluateMaxConsecutiveShifts(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int emp = 0; emp < num_employees; emp++) {
        const Staff& worker = instance.getStaff(emp);
        int consecutive_count = 0;
        
        for (int day = 0; day < horizon; day++) {
            if (schedule.getAssignment(emp, day) != 0) {
                consecutive_count++;
                if (consecutive_count > worker.MaxConsecutiveShifts) {
                    penalty -= 10;
                }
            } else {
                consecutive_count = 0;
            }
        }
    }
    return penalty;
}

int HardConstraints::evaluateMinConsecutiveShifts(const Schedule& schedule) const {
    // This function actually penalizes excessive consecutive days off
    // (the original naming is misleading)
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int emp = 0; emp < num_employees; emp++) {
        const Staff& worker = instance.getStaff(emp);
        int consecutive_days_off = 0;
        
        for (int day = 0; day < horizon; day++) {
            if (schedule.getAssignment(emp, day) == 0) {
                consecutive_days_off++;
                if (consecutive_days_off > worker.MinConsecutiveShifts) {
                    penalty -= 80;
                }
            } else {
                consecutive_days_off = 0;
            }
        }
    }
    return penalty;
}

int HardConstraints::evaluateMinConsecutiveDaysOff(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int emp = 0; emp < num_employees; emp++) {
        const Staff& worker = instance.getStaff(emp);
        bool in_work_period = false;
        int work_period_length = 0;
        
        for (int day = 0; day < horizon; day++) {
            if (schedule.getAssignment(emp, day) != 0) {
                if (!in_work_period) {
                    in_work_period = true;
                    work_period_length = 1;
                } else {
                    work_period_length++;
                }
            } else {
                if (in_work_period) {
                    // End of work period, check if it meets minimum
                    if (work_period_length < worker.MinConsecutiveShifts && work_period_length > 0) {
                        penalty -= 50;
                    }
                    in_work_period = false;
                    work_period_length = 0;
                }
            }
        }
        
        // Check final work period
        if (in_work_period && work_period_length < worker.MinConsecutiveShifts && work_period_length > 0) {
            penalty -= 50;
        }
    }
    return penalty;
}

int HardConstraints::evaluateMaxWeekendsWorked(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    
    for (int emp = 0; emp < num_employees; emp++) {
        const Staff& worker = instance.getStaff(emp);
        int weekend_count = countWeekendsWorked(schedule, emp);
        
        if (weekend_count > worker.MaxWeekends) {
            penalty -= 100 * weekend_count;
        }
    }
    return penalty;
}

int HardConstraints::evaluatePreAssignedDaysOff(const Schedule& schedule) const {
    int penalty = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    const auto& days_off_list = instance.getDaysOff();
    
    for (int emp = 0; emp < num_employees; emp++) {
        const Staff& worker = instance.getStaff(emp);
        
        // Find days off for this employee
        for (const auto& days_off : days_off_list) {
            if (days_off.EmployeeID == worker.ID) {
                for (const auto& day_str : days_off.DayIndexes) {
                    int day_index = std::stoi(day_str);
                    if (day_index >= 0 && day_index < horizon) {
                        if (schedule.getAssignment(emp, day_index) != 0) {
                            penalty -= 1000; // Very high penalty for violating mandatory days off
                        }
                    }
                }
                break;
            }
        }
    }
    return penalty;
}

// Aggregate evaluation methods

int HardConstraints::evaluateAll(const Schedule& schedule) const {
    int total_penalty = 0;
    
    // Only include constraints that exist in the original implementation
    total_penalty += evaluateMaxShiftsPerType(schedule);           // sumOfShift
    total_penalty += evaluateWorkingTimeConstraints(schedule);     // ShiftTimesSum  
    total_penalty += evaluateMaxConsecutiveShifts(schedule);       // maxConsecutiveShifts
    total_penalty += evaluateMinConsecutiveShifts(schedule);       // minConsecutiveShifts (days off)
    total_penalty += evaluateMaxWeekendsWorked(schedule);          // MaxConsecutiveWeekendWork
    total_penalty += evaluatePreAssignedDaysOff(schedule);         // MustDayoff
    total_penalty += evaluateShiftRotation(schedule);              // CantFollowRestriction
    
    return total_penalty;
}

bool HardConstraints::isFeasible(const Schedule& schedule) const {
    return evaluateAll(schedule) == 0;
}

int HardConstraints::evaluateEmployee(const Schedule& schedule, int employee) const {
    // Create a temporary schedule with only this employee's assignments
    Schedule temp_schedule(1, schedule.getHorizonDays());
    for (int day = 0; day < schedule.getHorizonDays(); day++) {
        temp_schedule.setAssignment(0, day, schedule.getAssignment(employee, day));
    }
    
    // Note: This is a simplified version - full implementation would need
    // to properly handle employee-specific constraints
    return evaluateAll(temp_schedule);
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