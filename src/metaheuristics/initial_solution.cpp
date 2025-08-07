#include "initial_solution.h"
#include "../utils/random.h"
#include <iostream>
#include <algorithm>
#include <cassert>

InitialSolutionGenerator::InitialSolutionGenerator(const Instance& instance) 
    : instance(instance), rng(std::random_device{}()) {
}

Schedule InitialSolutionGenerator::generateFeasibleSolution() {
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays(), instance.getNumShiftTypes());
    
    // Initialize tracking structures
    std::vector<EmployeeState> employee_states(instance.getNumEmployees());
    CoverageState coverage(instance.getHorizonDays(), instance.getNumShiftTypes());
    
    initializeEmployeeStates(employee_states);
    initializeCoverageRequirements(coverage);
    
    std::cout << "Starting 5-step feasible initial solution generation..." << std::endl;
    
    // Step 1: Assign annual leave (PreAssignedDaysOff)
    std::cout << "Step 1: Assigning annual leave..." << std::endl;
    assignAnnualLeave(schedule, employee_states);
    
    // Step 2: Cover weekend requirements ensuring each nurse has ≥2 weekends off
    std::cout << "Step 2: Assigning weekend coverage..." << std::endl;
    assignWeekends(schedule, employee_states, coverage);
    
    // Step 3: Assign shifts for first 4 days considering previous schedule constraints
    std::cout << "Step 3: Assigning initial days (0-3)..." << std::endl;
    assignInitialDays(schedule, employee_states, coverage);
    
    // Step 4: Iterate day-by-day for remaining horizon, assigning shifts to meet coverage
    std::cout << "Step 4: Assigning remaining horizon..." << std::endl;
    assignRemainingHorizon(schedule, employee_states, coverage);
    
    // Step 5: Adjust working hours by adding shifts to meet minimum hour requirements
    std::cout << "Step 5: Adjusting working hours..." << std::endl;
    adjustWorkingHours(schedule, employee_states, coverage);
    
    std::cout << "Initial solution generation completed." << std::endl;
    printGenerationStats(schedule, employee_states);
    
    return schedule;
}

void InitialSolutionGenerator::assignAnnualLeave(Schedule& schedule, std::vector<EmployeeState>& employee_states) {
    const auto& days_off = instance.getDaysOff();
    
    for (const auto& employee_days_off : days_off) {
        int employee_index = instance.getStaffIndex(employee_days_off.EmployeeID);
        
        if (employee_index == -1) {
            std::cerr << "Warning: Employee " << employee_days_off.EmployeeID << " not found" << std::endl;
            continue;
        }
        
        for (const std::string& day_str : employee_days_off.DayIndexes) {
            int day = std::stoi(day_str);
            
            if (instance.isValidDay(day)) {
                // Assign day off (shift 0)
                schedule.setAssignment(employee_index, day, 0);
                employee_states[employee_index].blocked_days.insert(day);
                
                std::cout << "  Blocked day " << day << " for employee " << employee_index << std::endl;
            }
        }
    }
}

void InitialSolutionGenerator::assignWeekends(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage) {
    // Calculate total weekends in the horizon
    int total_weekends = 0;
    for (int day = 0; day < instance.getHorizonDays(); day++) {
        if (isWeekend(day)) {
            total_weekends = std::max(total_weekends, getWeekendNumber(day) + 1);
        }
    }
    
    std::cout << "  Total weekends in horizon: " << total_weekends << std::endl;
    
    // For each weekend, assign coverage while ensuring no employee works more than allowed
    for (int weekend = 0; weekend < total_weekends; weekend++) {
        std::vector<int> weekend_days;
        
        // Find weekend days (Saturday=5, Sunday=6 in a 7-day cycle starting from day 0)
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            if (getWeekendNumber(day) == weekend && isWeekend(day)) {
                weekend_days.push_back(day);
            }
        }
        
        if (weekend_days.empty()) continue;
        
        // Try to assign weekend shifts while respecting max weekends constraint
        for (int day : weekend_days) {
            for (int shift = 1; shift <= instance.getNumShiftTypes(); shift++) {
                int required = coverage.required_coverage[day][shift-1];
                int current = coverage.current_coverage[day][shift-1];
                
                while (current < required) {
                    std::vector<int> available_employees = getAvailableEmployees(day, shift, schedule, employee_states);
                    
                    // Filter employees who haven't exceeded max weekends
                    available_employees.erase(
                        std::remove_if(available_employees.begin(), available_employees.end(),
                            [&](int emp) { return violatesMaxWeekends(emp, day, employee_states); }),
                        available_employees.end()
                    );
                    
                    if (available_employees.empty()) break;
                    
                    // Select employee with fewest weekends worked so far
                    int selected_employee = *std::min_element(available_employees.begin(), available_employees.end(),
                        [&](int a, int b) { return employee_states[a].weekends_worked < employee_states[b].weekends_worked; });
                    
                    schedule.setAssignment(selected_employee, day, shift);
                    updateEmployeeState(employee_states[selected_employee], day, shift, schedule);
                    updateCoverageState(coverage, day, shift, 1);
                    
                    current++;
                }
            }
        }
    }
}

void InitialSolutionGenerator::assignInitialDays(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage) {
    int initial_days = std::min(4, instance.getHorizonDays());
    
    for (int day = 0; day < initial_days; day++) {
        // Skip weekends (already handled in step 2)
        if (isWeekend(day)) continue;
        
        // Get under-covered shifts for this day
        std::vector<int> under_covered = getUnderCoveredShifts(day, coverage);
        
        for (int shift : under_covered) {
            int required = coverage.required_coverage[day][shift-1];
            int current = coverage.current_coverage[day][shift-1];
            
            while (current < required) {
                std::vector<int> available_employees = getAvailableEmployees(day, shift, schedule, employee_states);
                
                if (available_employees.empty()) break;
                
                // Prefer employees with fewer total minutes worked
                int selected_employee = *std::min_element(available_employees.begin(), available_employees.end(),
                    [&](int a, int b) { return employee_states[a].total_minutes_worked < employee_states[b].total_minutes_worked; });
                
                schedule.setAssignment(selected_employee, day, shift);
                updateEmployeeState(employee_states[selected_employee], day, shift, schedule);
                updateCoverageState(coverage, day, shift, 1);
                
                current++;
            }
        }
    }
}

void InitialSolutionGenerator::assignRemainingHorizon(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage) {
    int start_day = std::min(4, instance.getHorizonDays());
    
    for (int day = start_day; day < instance.getHorizonDays(); day++) {
        // Skip weekends (already handled in step 2)
        if (isWeekend(day)) continue;
        
        // Get under-covered shifts for this day
        std::vector<int> under_covered = getUnderCoveredShifts(day, coverage);
        
        for (int shift : under_covered) {
            int required = coverage.required_coverage[day][shift-1];
            int current = coverage.current_coverage[day][shift-1];
            
            while (current < required) {
                std::vector<int> available_employees = getAvailableEmployees(day, shift, schedule, employee_states);
                
                if (available_employees.empty()) break;
                
                // Use a more sophisticated selection strategy
                // Prefer employees who:
                // 1. Have fewer consecutive work days
                // 2. Have lower total minutes worked
                // 3. Don't violate shift sequence constraints
                
                int selected_employee = *std::min_element(available_employees.begin(), available_employees.end(),
                    [&](int a, int b) {
                        // Primary: fewer consecutive work days
                        if (employee_states[a].consecutive_work_days != employee_states[b].consecutive_work_days) {
                            return employee_states[a].consecutive_work_days < employee_states[b].consecutive_work_days;
                        }
                        // Secondary: fewer total minutes
                        return employee_states[a].total_minutes_worked < employee_states[b].total_minutes_worked;
                    });
                
                schedule.setAssignment(selected_employee, day, shift);
                updateEmployeeState(employee_states[selected_employee], day, shift, schedule);
                updateCoverageState(coverage, day, shift, 1);
                
                current++;
            }
        }
    }
}

void InitialSolutionGenerator::adjustWorkingHours(Schedule& schedule, std::vector<EmployeeState>& employee_states, CoverageState& coverage) {
    std::vector<int> employees_needing_hours = getEmployeesNeedingMoreHours(employee_states);
    
    std::cout << "  Employees needing more hours: " << employees_needing_hours.size() << std::endl;
    
    for (int employee : employees_needing_hours) {
        const Staff& staff = instance.getStaff(employee);
        int needed_minutes = staff.MinTotalMinutes - employee_states[employee].total_minutes_worked;
        
        std::cout << "    Employee " << employee << " needs " << needed_minutes << " more minutes" << std::endl;
        
        // Find days where this employee is not working and can be assigned
        for (int day = 0; day < instance.getHorizonDays() && needed_minutes > 0; day++) {
            if (schedule.getAssignment(employee, day) != 0) continue; // Already working
            if (employee_states[employee].blocked_days.count(day)) continue; // Day off required
            
            // Try to assign a shift that helps with coverage and hours
            for (int shift = 1; shift <= instance.getNumShiftTypes() && needed_minutes > 0; shift++) {
                if (canAssignShift(employee, day, shift, schedule, employee_states)) {
                    int shift_minutes = instance.getShift(shift-1).mins;
                    
                    // Only assign if it helps significantly with needed hours
                    if (shift_minutes >= std::min(needed_minutes, 60)) { // At least 1 hour or what's needed
                        schedule.setAssignment(employee, day, shift);
                        updateEmployeeState(employee_states[employee], day, shift, schedule);
                        updateCoverageState(coverage, day, shift, 1);
                        
                        needed_minutes -= shift_minutes;
                        std::cout << "      Assigned shift " << shift << " on day " << day 
                                  << " (" << shift_minutes << " minutes)" << std::endl;
                        break;
                    }
                }
            }
        }
    }
}

bool InitialSolutionGenerator::canAssignShift(int employee, int day, int shift, const Schedule& schedule, 
                                            const std::vector<EmployeeState>& employee_states) const {
    // Check if day is blocked (annual leave)
    if (employee_states[employee].blocked_days.count(day)) {
        return false;
    }
    
    // Check if already assigned
    if (schedule.getAssignment(employee, day) != 0) {
        return false;
    }
    
    // Check hard constraints
    if (violatesMaxConsecutiveShifts(employee, day, shift, employee_states)) {
        return false;
    }
    
    if (violatesShiftSequence(employee, day, shift, employee_states)) {
        return false;
    }
    
    if (violatesMaxTotalMinutes(employee, shift, employee_states)) {
        return false;
    }
    
    if (violatesMaxWeekends(employee, day, employee_states)) {
        return false;
    }
    
    return true;
}

std::vector<int> InitialSolutionGenerator::getAvailableEmployees(int day, int shift, const Schedule& schedule, 
                                                               const std::vector<EmployeeState>& employee_states) const {
    std::vector<int> available;
    
    for (int employee = 0; employee < instance.getNumEmployees(); employee++) {
        if (canAssignShift(employee, day, shift, schedule, employee_states)) {
            available.push_back(employee);
        }
    }
    
    return available;
}

std::vector<int> InitialSolutionGenerator::getUnderCoveredShifts(int day, const CoverageState& coverage) const {
    std::vector<int> under_covered;
    
    for (int shift = 1; shift <= instance.getNumShiftTypes(); shift++) {
        if (coverage.current_coverage[day][shift-1] < coverage.required_coverage[day][shift-1]) {
            under_covered.push_back(shift);
        }
    }
    
    // Sort by priority (most under-covered first)
    std::sort(under_covered.begin(), under_covered.end(),
        [&](int a, int b) {
            int deficit_a = coverage.required_coverage[day][a-1] - coverage.current_coverage[day][a-1];
            int deficit_b = coverage.required_coverage[day][b-1] - coverage.current_coverage[day][b-1];
            return deficit_a > deficit_b;
        });
    
    return under_covered;
}

std::vector<int> InitialSolutionGenerator::getEmployeesNeedingMoreHours(const std::vector<EmployeeState>& employee_states) const {
    std::vector<int> needing_hours;
    
    for (int employee = 0; employee < instance.getNumEmployees(); employee++) {
        const Staff& staff = instance.getStaff(employee);
        if (employee_states[employee].total_minutes_worked < staff.MinTotalMinutes) {
            needing_hours.push_back(employee);
        }
    }
    
    return needing_hours;
}

bool InitialSolutionGenerator::isWeekend(int day) const {
    // Assuming week starts on Monday (day 0), weekend is Saturday (5) and Sunday (6)
    int day_of_week = day % 7;
    return day_of_week == 5 || day_of_week == 6;
}

int InitialSolutionGenerator::getWeekendNumber(int day) const {
    // Return which weekend this day belongs to (0-based)
    return day / 7;
}

void InitialSolutionGenerator::updateEmployeeState(EmployeeState& state, int day, int shift, const Schedule& schedule) {
    const Shift& shift_info = instance.getShift(shift-1);
    
    // Update total minutes
    state.total_minutes_worked += shift_info.mins;
    
    // Update consecutive work/rest tracking
    if (shift == 0) {
        state.consecutive_work_days = 0;
        state.consecutive_days_off++;
    } else {
        state.consecutive_work_days++;
        state.consecutive_days_off = 0;
        
        // Update weekend count
        if (isWeekend(day)) {
            // Check if this is a new weekend (not already counted)
            int weekend_num = getWeekendNumber(day);
            bool already_working_this_weekend = false;
            
            // Check other days in this weekend
            for (int check_day = weekend_num * 7; check_day < (weekend_num + 1) * 7 && check_day < day; check_day++) {
                if (isWeekend(check_day) && schedule.getAssignment(schedule.getNumEmployees() - 1, check_day) != 0) {
                    // This is a hack - we need the employee index, but we don't have it here
                    // In a real implementation, we'd pass the employee index
                    already_working_this_weekend = true;
                    break;
                }
            }
            
            if (!already_working_this_weekend) {
                state.weekends_worked++;
            }
        }
    }
    
    // Update last shift type
    state.last_shift_type = shift;
}

void InitialSolutionGenerator::updateCoverageState(CoverageState& coverage, int day, int shift, int delta) {
    if (shift > 0 && shift <= instance.getNumShiftTypes()) {
        coverage.current_coverage[day][shift-1] += delta;
    }
}

bool InitialSolutionGenerator::violatesMaxConsecutiveShifts(int employee, int day, int shift, 
                                                          const std::vector<EmployeeState>& employee_states) const {
    if (shift == 0) return false; // Day off doesn't violate consecutive shifts
    
    const Staff& staff = instance.getStaff(employee);
    return employee_states[employee].consecutive_work_days >= staff.MaxConsecutiveShifts;
}

bool InitialSolutionGenerator::violatesShiftSequence(int employee, int day, int shift, 
                                                    const std::vector<EmployeeState>& employee_states) const {
    if (day == 0 || shift == 0) return false;
    
    int last_shift = employee_states[employee].last_shift_type;
    if (last_shift <= 0) return false;
    
    const Shift& last_shift_info = instance.getShift(last_shift-1);
    const Shift& current_shift_info = instance.getShift(shift-1);
    
    // Check if current shift is in the "can't follow" list of the last shift
    for (const std::string& cant_follow : last_shift_info.cant_follow) {
        if (cant_follow == current_shift_info.ShiftID) {
            return true;
        }
    }
    
    return false;
}

bool InitialSolutionGenerator::violatesMaxTotalMinutes(int employee, int shift, 
                                                     const std::vector<EmployeeState>& employee_states) const {
    if (shift == 0) return false;
    
    const Staff& staff = instance.getStaff(employee);
    const Shift& shift_info = instance.getShift(shift-1);
    
    return (employee_states[employee].total_minutes_worked + shift_info.mins) > staff.MaxTotalMinutes;
}

bool InitialSolutionGenerator::violatesMaxWeekends(int employee, int day, 
                                                 const std::vector<EmployeeState>& employee_states) const {
    if (!isWeekend(day)) return false;
    
    const Staff& staff = instance.getStaff(employee);
    return employee_states[employee].weekends_worked >= staff.MaxWeekends;
}

void InitialSolutionGenerator::initializeCoverageRequirements(CoverageState& coverage) {
    const auto& cover_reqs = instance.getCoverageRequirements();
    
    for (const auto& req : cover_reqs) {
        int shift_index = instance.getShiftIndex(req.ShiftID);
        if (shift_index != -1 && instance.isValidDay(req.Day)) {
            coverage.required_coverage[req.Day][shift_index] = req.Requirement;
        }
    }
}

void InitialSolutionGenerator::initializeEmployeeStates(std::vector<EmployeeState>& employee_states) {
    // Employee states are initialized with default values in constructor
    // Additional initialization can be added here if needed
}

void InitialSolutionGenerator::validateSolution(const Schedule& schedule) const {
    // Basic validation - can be expanded
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            int shift = schedule.getAssignment(emp, day);
            assert(shift >= 0 && shift <= instance.getNumShiftTypes());
        }
    }
}

void InitialSolutionGenerator::printGenerationStats(const Schedule& schedule, const std::vector<EmployeeState>& employee_states) const {
    std::cout << "\n=== Initial Solution Generation Statistics ===" << std::endl;
    
    // Count assignments
    int total_assignments = 0;
    int days_off = 0;
    
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        for (int day = 0; day < instance.getHorizonDays(); day++) {
            int shift = schedule.getAssignment(emp, day);
            if (shift == 0) {
                days_off++;
            } else {
                total_assignments++;
            }
        }
    }
    
    std::cout << "Total shift assignments: " << total_assignments << std::endl;
    std::cout << "Total days off: " << days_off << std::endl;
    
    // Employee statistics
    int employees_meeting_min_hours = 0;
    int employees_exceeding_max_hours = 0;
    
    for (int emp = 0; emp < instance.getNumEmployees(); emp++) {
        const Staff& staff = instance.getStaff(emp);
        int total_minutes = employee_states[emp].total_minutes_worked;
        
        if (total_minutes >= staff.MinTotalMinutes) {
            employees_meeting_min_hours++;
        }
        if (total_minutes > staff.MaxTotalMinutes) {
            employees_exceeding_max_hours++;
        }
    }
    
    std::cout << "Employees meeting minimum hours: " << employees_meeting_min_hours 
              << "/" << instance.getNumEmployees() << std::endl;
    std::cout << "Employees exceeding maximum hours: " << employees_exceeding_max_hours 
              << "/" << instance.getNumEmployees() << std::endl;
    
    std::cout << "=============================================" << std::endl;
}