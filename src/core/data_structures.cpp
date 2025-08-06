#include "data_structures.h"
#include <random>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

Schedule::Schedule(int employees, int days) 
    : num_employees(employees), horizon_days(days), cache_valid(false) {
    assignments.resize(employees);
    for (int i = 0; i < employees; i++) {
        assignments[i].resize(days, 0);  // Initialize with 0 (no shift)
    }
    
    // Initialize cache structures
    shift_counts.resize(employees);
    total_minutes.resize(employees, 0);
}

Schedule::Schedule(const Schedule& other) 
    : num_employees(other.num_employees), horizon_days(other.horizon_days), 
      cache_valid(other.cache_valid) {
    assignments = other.assignments;
    shift_counts = other.shift_counts;
    total_minutes = other.total_minutes;
}

Schedule& Schedule::operator=(const Schedule& other) {
    if (this != &other) {
        num_employees = other.num_employees;
        horizon_days = other.horizon_days;
        assignments = other.assignments;
        cache_valid = other.cache_valid;
        shift_counts = other.shift_counts;
        total_minutes = other.total_minutes;
    }
    return *this;
}

void Schedule::setAssignment(int employee, int day, int shift) {
    if (employee >= 0 && employee < num_employees && 
        day >= 0 && day < horizon_days) {
        assignments[employee][day] = shift;
        invalidateCache();
    }
}

int Schedule::getAssignment(int employee, int day) const {
    if (employee >= 0 && employee < num_employees && 
        day >= 0 && day < horizon_days) {
        return assignments[employee][day];
    }
    return 0;  // Default to no shift
}

void Schedule::randomize(int max_shifts) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, max_shifts);
    
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            assignments[i][j] = dis(gen);
        }
    }
}

void Schedule::copyFrom(const Schedule& other) {
    if (num_employees == other.num_employees && 
        horizon_days == other.horizon_days) {
        assignments = other.assignments;
    }
}

// For compatibility with existing raw matrix code
int** Schedule::getRawMatrix() const {
    int** matrix = new int*[num_employees];
    for (int i = 0; i < num_employees; i++) {
        matrix[i] = new int[horizon_days];
        for (int j = 0; j < horizon_days; j++) {
            matrix[i][j] = assignments[i][j];
        }
    }
    return matrix;
}

void Schedule::setFromRawMatrix(int** matrix) {
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            assignments[i][j] = matrix[i][j];
        }
    }
    invalidateCache();
}

// Cache management methods
void Schedule::invalidateCache() const {
    cache_valid = false;
}

void Schedule::updateCache() const {
    if (cache_valid) return;
    
    // Reset cache structures
    for (int i = 0; i < num_employees; i++) {
        shift_counts[i].clear();
        total_minutes[i] = 0;
    }
    
    // Rebuild cache from current assignments
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            int shift = assignments[i][j];
            if (shift > 0) {
                shift_counts[i][shift]++;
            }
        }
    }
    
    cache_valid = true;
}

void Schedule::ensureCacheValid() const {
    if (!cache_valid) {
        updateCache();
    }
}

// Additional manipulation methods
void Schedule::clear() {
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            assignments[i][j] = 0;
        }
    }
    invalidateCache();
}

void Schedule::swapAssignments(int emp1, int day1, int emp2, int day2) {
    if (emp1 >= 0 && emp1 < num_employees && day1 >= 0 && day1 < horizon_days &&
        emp2 >= 0 && emp2 < num_employees && day2 >= 0 && day2 < horizon_days) {
        
        int temp = assignments[emp1][day1];
        assignments[emp1][day1] = assignments[emp2][day2];
        assignments[emp2][day2] = temp;
        invalidateCache();
    }
}

// Analysis methods
int Schedule::getShiftCount(int employee, int shift_type) const {
    if (employee < 0 || employee >= num_employees) return 0;
    
    ensureCacheValid();
    auto it = shift_counts[employee].find(shift_type);
    return (it != shift_counts[employee].end()) ? it->second : 0;
}

int Schedule::getTotalMinutes(int employee, const std::vector<int>& shift_durations) const {
    if (employee < 0 || employee >= num_employees) return 0;
    
    int total = 0;
    for (int day = 0; day < horizon_days; day++) {
        int shift = assignments[employee][day];
        if (shift > 0 && shift <= static_cast<int>(shift_durations.size())) {
            total += shift_durations[shift - 1];  // shift_durations is 0-indexed
        }
    }
    return total;
}

int Schedule::getConsecutiveShifts(int employee, int start_day) const {
    if (employee < 0 || employee >= num_employees || start_day < 0) return 0;
    
    int count = 0;
    for (int day = start_day; day < horizon_days; day++) {
        if (assignments[employee][day] != 0) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

int Schedule::getConsecutiveDaysOff(int employee, int start_day) const {
    if (employee < 0 || employee >= num_employees || start_day < 0) return 0;
    
    int count = 0;
    for (int day = start_day; day < horizon_days; day++) {
        if (assignments[employee][day] == 0) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

bool Schedule::isWorkingWeekend(int employee, int weekend_start_day) const {
    if (employee < 0 || employee >= num_employees || 
        weekend_start_day < 0 || weekend_start_day + 1 >= horizon_days) {
        return false;
    }
    
    return (assignments[employee][weekend_start_day] != 0 || 
            assignments[employee][weekend_start_day + 1] != 0);
}

// Coverage analysis
int Schedule::getCoverage(int day, int shift_type) const {
    if (day < 0 || day >= horizon_days) return 0;
    
    int coverage = 0;
    for (int emp = 0; emp < num_employees; emp++) {
        if (assignments[emp][day] == shift_type) {
            coverage++;
        }
    }
    return coverage;
}

std::vector<int> Schedule::getDailyCoverage(int day) const {
    std::vector<int> coverage;
    if (day < 0 || day >= horizon_days) return coverage;
    
    // Find max shift type to determine coverage vector size
    int max_shift = 0;
    for (int emp = 0; emp < num_employees; emp++) {
        max_shift = std::max(max_shift, assignments[emp][day]);
    }
    
    coverage.resize(max_shift + 1, 0);
    for (int emp = 0; emp < num_employees; emp++) {
        int shift = assignments[emp][day];
        if (shift >= 0 && shift <= max_shift) {
            coverage[shift]++;
        }
    }
    
    return coverage;
}

// Validation helpers
bool Schedule::isValidAssignment(int employee, int day, int shift) const {
    return (employee >= 0 && employee < num_employees && 
            day >= 0 && day < horizon_days && 
            shift >= 0);
}

bool Schedule::isEmpty() const {
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            if (assignments[i][j] != 0) {
                return false;
            }
        }
    }
    return true;
}

// Statistics
double Schedule::getUtilizationRate() const {
    int total_assignments = 0;
    int working_assignments = 0;
    
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            total_assignments++;
            if (assignments[i][j] != 0) {
                working_assignments++;
            }
        }
    }
    
    return total_assignments > 0 ? 
           static_cast<double>(working_assignments) / total_assignments : 0.0;
}

std::vector<int> Schedule::getWorkloadDistribution() const {
    std::vector<int> workload(num_employees, 0);
    
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            if (assignments[i][j] != 0) {
                workload[i]++;
            }
        }
    }
    
    return workload;
}

// Comparison and hashing
bool Schedule::operator==(const Schedule& other) const {
    return (num_employees == other.num_employees && 
            horizon_days == other.horizon_days && 
            assignments == other.assignments);
}

bool Schedule::operator!=(const Schedule& other) const {
    return !(*this == other);
}

size_t Schedule::hash() const {
    size_t hash_value = 0;
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            hash_value ^= std::hash<int>{}(assignments[i][j]) + 0x9e3779b9 + 
                         (hash_value << 6) + (hash_value >> 2);
        }
    }
    return hash_value;
}

// Serialization
std::string Schedule::toString() const {
    std::ostringstream oss;
    oss << "Schedule(" << num_employees << "x" << horizon_days << "):\n";
    
    for (int i = 0; i < num_employees; i++) {
        oss << "Employee " << std::setw(2) << i << ": ";
        for (int j = 0; j < horizon_days; j++) {
            oss << std::setw(2) << assignments[i][j];
            if (j < horizon_days - 1) oss << " ";
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::string Schedule::toCompactString() const {
    std::ostringstream oss;
    for (int i = 0; i < num_employees; i++) {
        for (int j = 0; j < horizon_days; j++) {
            oss << assignments[i][j];
            if (j < horizon_days - 1) oss << ",";
        }
        if (i < num_employees - 1) oss << ";";
    }
    return oss.str();
}

void Schedule::fromString(const std::string& str) {
    // Simple implementation for compact string format
    std::istringstream iss(str);
    std::string employee_line;
    int emp = 0;
    
    while (std::getline(iss, employee_line, ';') && emp < num_employees) {
        std::istringstream day_stream(employee_line);
        std::string day_value;
        int day = 0;
        
        while (std::getline(day_stream, day_value, ',') && day < horizon_days) {
            assignments[emp][day] = std::stoi(day_value);
            day++;
        }
        emp++;
    }
    
    invalidateCache();
}

// Memory management
size_t Schedule::getMemoryFootprint() const {
    size_t base_size = sizeof(Schedule);
    size_t assignments_size = assignments.size() * sizeof(std::vector<int>);
    
    for (const auto& emp_schedule : assignments) {
        assignments_size += emp_schedule.size() * sizeof(int);
    }
    
    size_t cache_size = shift_counts.size() * sizeof(std::vector<int>) + 
                       total_minutes.size() * sizeof(int);
    
    return base_size + assignments_size + cache_size;
}

void Schedule::shrinkToFit() {
    assignments.shrink_to_fit();
    for (auto& emp_schedule : assignments) {
        emp_schedule.shrink_to_fit();
    }
    shift_counts.shrink_to_fit();
    total_minutes.shrink_to_fit();
}