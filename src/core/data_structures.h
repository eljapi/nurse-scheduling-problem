#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <vector>
#include <string>
#include <unordered_map>

// Forward declarations
class Instance;

#include <string>
#include <unordered_map>

// Forward declarations
class Instance;

/**
 * Represents a staff member with all their constraints and preferences
 */
struct Staff {
    std::string ID;
    std::vector<std::string> MaxShifts;  // no hay turno 0 en esta representacion
    int MaxTotalMinutes;
    int MinTotalMinutes;
    int MaxConsecutiveShifts;
    int MinConsecutiveShifts;
    int MinConsecutiveDaysOff;
    int MaxWeekends;
};

/**
 * Represents a shift type with duration and restrictions
 */
struct Shift {
    std::string ShiftID;
    int mins;
    std::vector<std::string> cant_follow;
};

/**
 * Represents days off requirements for an employee
 */
struct DaysOff {
    std::string EmployeeID;
    std::vector<std::string> DayIndexes;
};

/**
 * Represents a shift-on request (employee wants to work a specific shift)
 */
struct ShiftOnRequest {
    std::string EmployeeID;
    int Day;
    std::string ShiftID;
    int Weight;
};

/**
 * Represents a shift-off request (employee wants to avoid a specific shift)
 */
struct ShiftOffRequest {
    std::string EmployeeID;
    int Day;
    std::string ShiftID;
    int Weight;
};

/**
 * Represents coverage requirements for a specific day and shift
 */
struct Cover {
    int Day;
    std::string ShiftID;
    int Requirement;
    int Weight_for_under;
    int Weight_for_over;
};

/**
 * Represents a complete schedule solution with optimized operations
 */
class Schedule {
private:
    std::vector<std::vector<int>> assignments;  // [employee][day] = shift_id
    int num_employees;
    int horizon_days;
    int num_shift_types;
    
    // Cached data for performance
    mutable bool cache_valid;
    mutable std::vector<std::unordered_map<int, int>> shift_counts;  // [employee][shift_type] = count
    mutable std::vector<int> total_minutes;                          // [employee] = total minutes worked
    
    // Helper methods
    void invalidateCache() const;
    void updateCache() const;
    void ensureCacheValid() const;
    
public:
    Schedule(int employees, int days, int shift_types);
    Schedule(const Schedule& other);
    Schedule& operator=(const Schedule& other);
    
    // Basic assignment operations
    void setAssignment(int employee, int day, int shift);
    int getAssignment(int employee, int day) const;
    
    // Getters
    int getNumEmployees() const { return num_employees; }
    int getHorizonDays() const { return horizon_days; }
    int getNumShiftTypes() const { return num_shift_types; }
    
    // Schedule manipulation
    void randomize(int max_shifts);
    void copyFrom(const Schedule& other);
    void clear();
    void swapAssignments(int emp1, int day1, int emp2, int day2);
    
    // Analysis methods (optimized with caching)
    int getShiftCount(int employee, int shift_type) const;
    int getTotalMinutes(int employee, const std::vector<int>& shift_durations) const;
    int getConsecutiveShifts(int employee, int start_day) const;
    int getConsecutiveDaysOff(int employee, int start_day) const;
    bool isWorkingWeekend(int employee, int weekend_start_day) const;
    
    // Coverage analysis
    int getCoverage(int day, int shift_type) const;
    std::vector<int> getDailyCoverage(int day) const;
    
    // Validation helpers
    bool isValidAssignment(int employee, int day, int shift) const;
    bool isEmpty() const;
    
    // Statistics
    double getUtilizationRate() const;
    std::vector<int> getWorkloadDistribution() const;
    
    // Comparison and hashing
    bool operator==(const Schedule& other) const;
    bool operator!=(const Schedule& other) const;
    size_t hash() const;
    
    // Serialization
    std::string toString() const;
    std::string toCompactString() const;
    void fromString(const std::string& str);
    
    // Memory management
    size_t getMemoryFootprint() const;
    void shrinkToFit();
    
    // For compatibility with existing code
    int** getRawMatrix() const;
    void setFromRawMatrix(int** matrix);
    
    // Iterator support for range-based loops
    class EmployeeScheduleView {
    private:
        const Schedule& schedule;
        int employee_id;
    public:
        EmployeeScheduleView(const Schedule& sched, int emp) : schedule(sched), employee_id(emp) {}
        int operator[](int day) const { return schedule.getAssignment(employee_id, day); }
        int size() const { return schedule.getHorizonDays(); }
    };
    
    EmployeeScheduleView getEmployeeSchedule(int employee) const {
        return EmployeeScheduleView(*this, employee);
    }
};

#endif // DATA_STRUCTURES_H
