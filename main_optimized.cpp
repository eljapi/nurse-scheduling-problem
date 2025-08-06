/**
 * NSP Optimized - Main entry point
 * 
 * This is a placeholder main file that will eventually replace main.cpp
 * For now, it just demonstrates the new modular structure
 */

#include "src/core/instance_parser.h"
#include "src/core/data_structures.h"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <instance_file> <iterations>" << std::endl;
        return 1;
    }
    
    std::string instance_file = "nsp_instancias/instances1_24/" + std::string(argv[1]);
    int iterations = std::stoi(argv[2]);
    
    std::cout << "NSP Optimized Version" << std::endl;
    std::cout << "Instance: " << instance_file << std::endl;
    std::cout << "Iterations: " << iterations << std::endl;
    
    // Parse instance
    InstanceParser parser;
    int horizon;
    std::vector<Staff> workers;
    std::vector<Shift> shifts;
    std::vector<DaysOff> days_off;
    std::vector<ShiftOnRequest> shift_on_requests;
    std::vector<ShiftOffRequest> shift_off_requests;
    std::vector<Cover> cover_requirements;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    if (!parser.parseInstance(instance_file, horizon, workers, shifts, 
                             days_off, shift_on_requests, 
                             shift_off_requests, cover_requirements)) {
        std::cerr << "Error: Failed to parse instance file" << std::endl;
        return 1;
    }
    
    auto parse_time = std::chrono::high_resolution_clock::now();
    auto parse_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parse_time - start_time);
    
    std::cout << "Parsed successfully:" << std::endl;
    std::cout << "  Horizon: " << horizon << " days" << std::endl;
    std::cout << "  Workers: " << workers.size() << std::endl;
    std::cout << "  Shifts: " << shifts.size() << std::endl;
    std::cout << "  Days off: " << days_off.size() << std::endl;
    std::cout << "  Shift requests: " << shift_on_requests.size() << std::endl;
    std::cout << "  Shift off requests: " << shift_off_requests.size() << std::endl;
    std::cout << "  Cover requirements: " << cover_requirements.size() << std::endl;
    std::cout << "  Parse time: " << parse_duration.count() << "ms" << std::endl;
    
    // Create initial schedule
    Schedule schedule(workers.size(), horizon);
    schedule.randomize(shifts.size());
    
    std::cout << "\nInitial schedule created with " << workers.size() 
              << " employees and " << horizon << " days" << std::endl;
    
    // TODO: Implement constraint evaluation and optimization
    std::cout << "\nNote: Full optimization not yet implemented." << std::endl;
    std::cout << "This is a modular structure demonstration." << std::endl;
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Total execution time: " << total_duration.count() << "ms" << std::endl;
    
    return 0;
}