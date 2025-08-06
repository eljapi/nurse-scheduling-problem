/**
 * NSP Optimized - Main entry point
 * 
 * Uses the new modular Instance class for optimized data loading and access
 */

#include "src/core/instance.h"
#include "src/core/data_structures.h"
#include <iostream>
#include <chrono>
#include <algorithm>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <instance_file> <iterations>" << std::endl;
        return 1;
    }
    
    std::string instance_file = "nsp_instancias/instances1_24/" + std::string(argv[1]);
    int iterations = std::stoi(argv[2]);
    
    std::cout << "NSP Optimized Version (with Instance class)" << std::endl;
    std::cout << "Instance: " << instance_file << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Load instance using the new Instance class
    Instance instance;
    if (!instance.loadFromFile(instance_file)) {
        std::cerr << "Error: Failed to load instance file" << std::endl;
        return 1;
    }
    
    auto parse_time = std::chrono::high_resolution_clock::now();
    auto parse_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parse_time - start_time);
    
    std::cout << "\nInstance loaded successfully:" << std::endl;
    std::cout << "  Horizon: " << instance.getHorizonDays() << " days" << std::endl;
    std::cout << "  Employees: " << instance.getNumEmployees() << std::endl;
    std::cout << "  Shift types: " << instance.getNumShiftTypes() << std::endl;
    std::cout << "  Days off: " << instance.getDaysOff().size() << std::endl;
    std::cout << "  Shift-on requests: " << instance.getShiftOnRequests().size() << std::endl;
    std::cout << "  Shift-off requests: " << instance.getShiftOffRequests().size() << std::endl;
    std::cout << "  Coverage requirements: " << instance.getCoverageRequirements().size() << std::endl;
    std::cout << "  Parse time: " << parse_duration.count() << "ms" << std::endl;
    std::cout << "  Memory footprint: " << instance.getMemoryFootprint() << " bytes" << std::endl;
    
    // Create initial schedule using Instance data
    Schedule schedule(instance.getNumEmployees(), instance.getHorizonDays());
    schedule.randomize(instance.getNumShiftTypes());
    
    std::cout << "\nInitial schedule created with " << instance.getNumEmployees() 
              << " employees and " << instance.getHorizonDays() << " days" << std::endl;
    
    // Demonstrate optimized access
    std::cout << "\nDemonstrating optimized data access:" << std::endl;
    
    // Show staff lookup by ID
    try {
        const Staff& staff_a = instance.getStaffById("A");
        std::cout << "  Staff A: max=" << staff_a.MaxTotalMinutes 
                  << " min=" << staff_a.MinTotalMinutes << " minutes" << std::endl;
        
        // Show availability check
        bool available_day_0 = instance.isEmployeeAvailable(0, 0);
        std::cout << "  Employee 0 available on day 0: " << (available_day_0 ? "Yes" : "No") << std::endl;
        
        // Show coverage requirement
        if (!instance.getShifts().empty()) {
            const std::string& shift_id = instance.getShifts()[0].ShiftID;
            int coverage = instance.getCoverageRequirement(0, shift_id);
            std::cout << "  Coverage requirement for day 0, shift " << shift_id << ": " << coverage << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "  Error in data access: " << e.what() << std::endl;
    }
    
    // Demonstrate Schedule class functionality
    std::cout << "\nDemonstrating Schedule class operations:" << std::endl;
    
    // Show schedule statistics
    double utilization = schedule.getUtilizationRate();
    std::cout << "  Initial utilization rate: " << (utilization * 100) << "%" << std::endl;
    
    // Show memory footprint
    size_t schedule_memory = schedule.getMemoryFootprint();
    std::cout << "  Schedule memory footprint: " << schedule_memory << " bytes" << std::endl;
    
    // Demonstrate raw matrix compatibility (for integration with existing code)
    int** raw_matrix = schedule.getRawMatrix();
    std::cout << "  Raw matrix compatibility: Available for legacy code integration" << std::endl;
    
    // Show first few assignments
    std::cout << "  Sample assignments (first 3 employees, first 5 days):" << std::endl;
    for (int emp = 0; emp < std::min(3, instance.getNumEmployees()); emp++) {
        std::cout << "    Employee " << emp << ": ";
        for (int day = 0; day < std::min(5, instance.getHorizonDays()); day++) {
            std::cout << raw_matrix[emp][day] << " ";
        }
        std::cout << std::endl;
    }
    
    // Clean up raw matrix
    for (int i = 0; i < instance.getNumEmployees(); i++) {
        delete[] raw_matrix[i];
    }
    delete[] raw_matrix;
    
    // Demonstrate schedule manipulation
    Schedule modified_schedule = schedule;  // Copy constructor
    modified_schedule.setAssignment(0, 0, 1);
    modified_schedule.setAssignment(0, 1, 2);
    
    int consecutive_shifts = modified_schedule.getConsecutiveShifts(0, 0);
    std::cout << "  After manual assignment: Employee 0 has " << consecutive_shifts 
              << " consecutive shifts starting from day 0" << std::endl;
    
    // TODO: Implement constraint evaluation and optimization
    std::cout << "\nNote: Full constraint evaluation and optimization not yet implemented." << std::endl;
    std::cout << "This demonstrates the new Schedule class replacing raw 2D arrays." << std::endl;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Total execution time: " << total_duration.count() << "ms" << std::endl;
    
    return 0;
}