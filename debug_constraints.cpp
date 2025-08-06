/**
 * Debug tool to compare original constraint functions with HardConstraints
 */

#include "src/constraints/hard_constraints.h"
#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include <iostream>

// Forward declarations of original functions (we'll need to extract them)
int sumOfShift_original(const Schedule& schedule, const Instance& instance);
int ShiftTimesSum_original(const Schedule& schedule, const Instance& instance);
int maxConsecutiveShifts_original(const Schedule& schedule, const Instance& instance);
int minConsecutiveShifts_original(const Schedule& schedule, const Instance& instance);
int MaxConsecutiveWeekendWork_original(const Schedule& schedule, const Instance& instance);
int MustDayoff_original(const Schedule& schedule, const Instance& instance);
int CantFollowRestriction_original(const Schedule& schedule, const Instance& instance);

// Implementations of original functions adapted to use Schedule and Instance
int sumOfShift_original(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int num_shifts = instance.getNumShiftTypes();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        
        for (int shift_type = 1; shift_type <= num_shifts; shift_type++) {
            int shift_count = schedule.getShiftCount(i, shift_type);
            
            if (shift_type - 1 < static_cast<int>(worker.MaxShifts.size())) {
                int max_shifts = std::stoi(worker.MaxShifts[shift_type - 1]);
                if (shift_count > max_shifts) {
                    score -= 10;
                }
            }
        }
    }
    return score;
}

int ShiftTimesSum_original(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        int total_minutes = 0;
        
        for (int shift_type = 1; shift_type <= instance.getNumShiftTypes(); shift_type++) {
            int shift_count = schedule.getShiftCount(i, shift_type);
            const Shift& shift_info = instance.getShift(shift_type - 1);
            total_minutes += shift_count * shift_info.mins;
        }
        
        if (total_minutes > worker.MaxTotalMinutes) {
            score -= 10;
        }
        if (total_minutes < worker.MinTotalMinutes) {
            score -= 10;
        }
    }
    return score;
}

int maxConsecutiveShifts_original(const Schedule& schedule, const Instance& instance) {
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

int minConsecutiveShifts_original(const Schedule& schedule, const Instance& instance) {
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

int MaxConsecutiveWeekendWork_original(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        int weekend_count = 0;
        
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

int MustDayoff_original(const Schedule& schedule, const Instance& instance) {
    int score = 0;
    int num_employees = schedule.getNumEmployees();
    int horizon = schedule.getHorizonDays();
    
    const auto& days_off_list = instance.getDaysOff();
    
    for (int i = 0; i < num_employees; i++) {
        const Staff& worker = instance.getStaff(i);
        
        for (const auto& days_off : days_off_list) {
            if (days_off.EmployeeID == worker.ID) {
                for (const auto& day_str : days_off.DayIndexes) {
                    int day_index = std::stoi(day_str);
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

int CantFollowRestriction_original(const Schedule& schedule, const Instance& instance) {
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

int main() {
    std::cout << "=== Constraint Functions Comparison ===" << std::endl;
    
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance1.txt")) {
        std::cerr << "Failed to load Instance1.txt" << std::endl;
        return 1;
    }
    
    HardConstraints constraints(instance);
    
    // Test with a random schedule
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    schedule.randomize(instance.getNumShiftTypes());
    
    std::cout << "Testing with random schedule..." << std::endl;
    
    // Compare each function
    std::cout << "\nFunction-by-function comparison:" << std::endl;
    
    int orig_sum = sumOfShift_original(schedule, instance);
    int new_sum = constraints.evaluateMaxShiftsPerType(schedule);
    std::cout << "sumOfShift:           Original=" << orig_sum << ", New=" << new_sum << " " << (orig_sum == new_sum ? "✓" : "✗") << std::endl;
    
    int orig_time = ShiftTimesSum_original(schedule, instance);
    int new_time = constraints.evaluateWorkingTimeConstraints(schedule);
    std::cout << "ShiftTimesSum:        Original=" << orig_time << ", New=" << new_time << " " << (orig_time == new_time ? "✓" : "✗") << std::endl;
    
    int orig_max = maxConsecutiveShifts_original(schedule, instance);
    int new_max = constraints.evaluateMaxConsecutiveShifts(schedule);
    std::cout << "maxConsecutiveShifts: Original=" << orig_max << ", New=" << new_max << " " << (orig_max == new_max ? "✓" : "✗") << std::endl;
    
    int orig_min = minConsecutiveShifts_original(schedule, instance);
    int new_min = constraints.evaluateMinConsecutiveShifts(schedule);
    std::cout << "minConsecutiveShifts: Original=" << orig_min << ", New=" << new_min << " " << (orig_min == new_min ? "✓" : "✗") << std::endl;
    
    int orig_weekend = MaxConsecutiveWeekendWork_original(schedule, instance);
    int new_weekend = constraints.evaluateMaxWeekendsWorked(schedule);
    std::cout << "MaxWeekendWork:       Original=" << orig_weekend << ", New=" << new_weekend << " " << (orig_weekend == new_weekend ? "✓" : "✗") << std::endl;
    
    int orig_dayoff = MustDayoff_original(schedule, instance);
    int new_dayoff = constraints.evaluatePreAssignedDaysOff(schedule);
    std::cout << "MustDayoff:           Original=" << orig_dayoff << ", New=" << new_dayoff << " " << (orig_dayoff == new_dayoff ? "✓" : "✗") << std::endl;
    
    int orig_follow = CantFollowRestriction_original(schedule, instance);
    int new_follow = constraints.evaluateShiftRotation(schedule);
    std::cout << "CantFollowRestriction:Original=" << orig_follow << ", New=" << new_follow << " " << (orig_follow == new_follow ? "✓" : "✗") << std::endl;
    
    // Total comparison
    int orig_total = orig_sum + orig_time + orig_max + orig_min + orig_weekend + orig_dayoff + orig_follow;
    int new_total = constraints.evaluateAll(schedule);
    
    std::cout << "\nTotal score comparison:" << std::endl;
    std::cout << "Original total: " << orig_total << std::endl;
    std::cout << "New total:      " << new_total << std::endl;
    std::cout << "Match: " << (orig_total == new_total ? "✓" : "✗") << std::endl;
    
    if (orig_total != new_total) {
        std::cout << "Difference: " << (new_total - orig_total) << std::endl;
    }
    
    return 0;
}
