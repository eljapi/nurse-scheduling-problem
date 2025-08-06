#include "instance.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

Instance::Instance() 
    : horizon_days(0), num_employees(0), num_shift_types(0) {
}

bool Instance::loadFromFile(const std::string& filename) {
    InstanceParser parser;
    
    // Clear existing data
    staff_members.clear();
    shift_types.clear();
    days_off_requirements.clear();
    shift_on_requests.clear();
    shift_off_requests.clear();
    coverage_requirements.clear();
    staff_id_to_index.clear();
    shift_id_to_index.clear();
    
    // Parse the instance file
    bool success = parser.parseInstance(filename, horizon_days, staff_members, shift_types,
                                       days_off_requirements, shift_on_requests,
                                       shift_off_requests, coverage_requirements);
    
    if (!success) {
        return false;
    }
    
    // Build optimized data structures
    precomputeData();
    buildLookupTables();
    
    return isValid();
}

void Instance::precomputeData() {
    num_employees = static_cast<int>(staff_members.size());
    num_shift_types = static_cast<int>(shift_types.size());
}

void Instance::buildLookupTables() {
    // Build staff ID to index mapping
    staff_id_to_index.clear();
    for (size_t i = 0; i < staff_members.size(); ++i) {
        staff_id_to_index[staff_members[i].ID] = static_cast<int>(i);
    }
    
    // Build shift ID to index mapping
    shift_id_to_index.clear();
    for (size_t i = 0; i < shift_types.size(); ++i) {
        shift_id_to_index[shift_types[i].ShiftID] = static_cast<int>(i);
    }
}

bool Instance::isValid() const {
    // Basic validation checks
    if (horizon_days <= 0) return false;
    if (staff_members.empty()) return false;
    if (shift_types.empty()) return false;
    
    // Check that all staff IDs are unique
    std::unordered_map<std::string, int> id_count;
    for (const auto& staff : staff_members) {
        if (staff.ID.empty()) return false;
        id_count[staff.ID]++;
        if (id_count[staff.ID] > 1) return false;
    }
    
    // Check that all shift IDs are unique
    id_count.clear();
    for (const auto& shift : shift_types) {
        if (shift.ShiftID.empty()) return false;
        id_count[shift.ShiftID]++;
        if (id_count[shift.ShiftID] > 1) return false;
    }
    
    return true;
}

const Staff& Instance::getStaff(int index) const {
    if (!isValidStaffIndex(index)) {
        throw std::out_of_range("Invalid staff index: " + std::to_string(index));
    }
    return staff_members[index];
}

const Staff& Instance::getStaffById(const std::string& id) const {
    int index = getStaffIndex(id);
    if (index == -1) {
        throw std::invalid_argument("Staff ID not found: " + id);
    }
    return staff_members[index];
}

const Shift& Instance::getShift(int index) const {
    if (!isValidShiftIndex(index)) {
        throw std::out_of_range("Invalid shift index: " + std::to_string(index));
    }
    return shift_types[index];
}

const Shift& Instance::getShiftById(const std::string& id) const {
    int index = getShiftIndex(id);
    if (index == -1) {
        throw std::invalid_argument("Shift ID not found: " + id);
    }
    return shift_types[index];
}

int Instance::getStaffIndex(const std::string& id) const {
    auto it = staff_id_to_index.find(id);
    return (it != staff_id_to_index.end()) ? it->second : -1;
}

int Instance::getShiftIndex(const std::string& id) const {
    auto it = shift_id_to_index.find(id);
    return (it != shift_id_to_index.end()) ? it->second : -1;
}

bool Instance::isValidStaffIndex(int index) const {
    return index >= 0 && index < num_employees;
}

bool Instance::isValidShiftIndex(int index) const {
    return index >= 0 && index < num_shift_types;
}

bool Instance::isValidDay(int day) const {
    return day >= 0 && day < horizon_days;
}

bool Instance::isEmployeeAvailable(int employee_index, int day) const {
    if (!isValidStaffIndex(employee_index) || !isValidDay(day)) {
        return false;
    }
    
    const std::string& employee_id = staff_members[employee_index].ID;
    
    // Check if this day is in the employee's days off
    for (const auto& days_off : days_off_requirements) {
        if (days_off.EmployeeID == employee_id) {
            for (const auto& day_str : days_off.DayIndexes) {
                try {
                    int off_day = std::stoi(day_str);
                    if (off_day == day) {
                        return false;
                    }
                } catch (const std::exception&) {
                    // Invalid day format, skip
                    continue;
                }
            }
        }
    }
    
    return true;
}

int Instance::getCoverageRequirement(int day, const std::string& shift_id) const {
    if (!isValidDay(day)) {
        return 0;
    }
    
    for (const auto& cover : coverage_requirements) {
        if (cover.Day == day && cover.ShiftID == shift_id) {
            return cover.Requirement;
        }
    }
    
    return 0; // No specific requirement found
}

void Instance::printSummary() const {
    std::cout << "=== Instance Summary ===" << std::endl;
    std::cout << "Horizon: " << horizon_days << " days" << std::endl;
    std::cout << "Employees: " << num_employees << std::endl;
    std::cout << "Shift types: " << num_shift_types << std::endl;
    std::cout << "Days off requirements: " << days_off_requirements.size() << std::endl;
    std::cout << "Shift-on requests: " << shift_on_requests.size() << std::endl;
    std::cout << "Shift-off requests: " << shift_off_requests.size() << std::endl;
    std::cout << "Coverage requirements: " << coverage_requirements.size() << std::endl;
    
    std::cout << "\nEmployees:" << std::endl;
    for (int i = 0; i < num_employees; ++i) {
        const auto& staff = staff_members[i];
        std::cout << "  " << staff.ID << " (max: " << staff.MaxTotalMinutes 
                  << " min, min: " << staff.MinTotalMinutes << " min)" << std::endl;
    }
    
    std::cout << "\nShift types:" << std::endl;
    for (int i = 0; i < num_shift_types; ++i) {
        const auto& shift = shift_types[i];
        std::cout << "  " << shift.ShiftID << " (" << shift.mins << " minutes)" << std::endl;
    }
}

size_t Instance::getMemoryFootprint() const {
    size_t total = sizeof(*this);
    
    // Add vector capacities
    total += staff_members.capacity() * sizeof(Staff);
    total += shift_types.capacity() * sizeof(Shift);
    total += days_off_requirements.capacity() * sizeof(DaysOff);
    total += shift_on_requests.capacity() * sizeof(ShiftOnRequest);
    total += shift_off_requests.capacity() * sizeof(ShiftOffRequest);
    total += coverage_requirements.capacity() * sizeof(Cover);
    
    // Add hash map overhead (approximate)
    total += staff_id_to_index.size() * (sizeof(std::string) + sizeof(int) + 16); // overhead
    total += shift_id_to_index.size() * (sizeof(std::string) + sizeof(int) + 16); // overhead
    
    return total;
}