#ifndef INSTANCE_H
#define INSTANCE_H

#include "data_structures.h"
#include "instance_parser.h"
#include <string>
#include <vector>
#include <unordered_map>

/**
 * Represents a complete NSP instance with optimized data access
 */
class Instance {
private:
    // Core instance data
    int horizon_days;
    std::vector<Staff> staff_members;
    std::vector<Shift> shift_types;
    std::vector<DaysOff> days_off_requirements;
    std::vector<ShiftOnRequest> shift_on_requests;
    std::vector<ShiftOffRequest> shift_off_requests;
    std::vector<Cover> coverage_requirements;
    
    // Optimized lookup structures
    std::unordered_map<std::string, int> staff_id_to_index;
    std::unordered_map<std::string, int> shift_id_to_index;
    
    // Pre-computed data for faster access
    int num_employees;
    int num_shift_types;
    
    // Helper methods for optimization
    void buildLookupTables();
    void precomputeData();
    
public:
    Instance();
    ~Instance() = default;
    
    // Loading methods
    bool loadFromFile(const std::string& filename);
    bool isValid() const;
    
    // Basic getters
    int getHorizonDays() const { return horizon_days; }
    int getNumEmployees() const { return num_employees; }
    int getNumShiftTypes() const { return num_shift_types; }
    
    // Data access methods (const references for efficiency)
    const std::vector<Staff>& getStaff() const { return staff_members; }
    const std::vector<Shift>& getShifts() const { return shift_types; }
    const std::vector<DaysOff>& getDaysOff() const { return days_off_requirements; }
    const std::vector<ShiftOnRequest>& getShiftOnRequests() const { return shift_on_requests; }
    const std::vector<ShiftOffRequest>& getShiftOffRequests() const { return shift_off_requests; }
    const std::vector<Cover>& getCoverageRequirements() const { return coverage_requirements; }
    
    // Individual item access
    const Staff& getStaff(int index) const;
    const Staff& getStaffById(const std::string& id) const;
    const Shift& getShift(int index) const;
    const Shift& getShiftById(const std::string& id) const;
    
    // Optimized lookup methods
    int getStaffIndex(const std::string& id) const;
    int getShiftIndex(const std::string& id) const;
    
    // Validation methods
    bool isValidStaffIndex(int index) const;
    bool isValidShiftIndex(int index) const;
    bool isValidDay(int day) const;
    
    // Utility methods for constraint checking
    bool isEmployeeAvailable(int employee_index, int day) const;
    int getCoverageRequirement(int day, const std::string& shift_id) const;
    
    // Statistics and info
    void printSummary() const;
    size_t getMemoryFootprint() const;
};

#endif // INSTANCE_H