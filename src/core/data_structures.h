#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <vector>
#include <string>

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
 * Represents a complete schedule solution
 */
class Schedule {
private:
    std::vector<std::vector<int>> assignments;  // [employee][day] = shift_id
    int num_employees;
    int horizon_days;
    
public:
    Schedule(int employees, int days);
    Schedule(const Schedule& other);
    Schedule& operator=(const Schedule& other);
    
    void setAssignment(int employee, int day, int shift);
    int getAssignment(int employee, int day) const;
    
    int getNumEmployees() const { return num_employees; }
    int getHorizonDays() const { return horizon_days; }
    
    void randomize(int max_shifts);
    void copyFrom(const Schedule& other);
    
    // For compatibility with existing code
    int** getRawMatrix() const;
    void setFromRawMatrix(int** matrix);
};

#endif // DATA_STRUCTURES_H