#include "test_runner.h"
#include "../src/core/instance.h"
#include "../src/core/data_structures.h"
#include "../src/constraints/constraint_evaluator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

// Function to parse the solution file and create a Schedule object
inline Schedule parseSolution(const std::string& solution_file, const Instance& instance) {
    int num_employees = instance.getNumEmployees();
    int horizon = instance.getHorizonDays();
    Schedule schedule(num_employees, horizon, instance.getNumShiftTypes());

    std::ifstream file(solution_file);
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("Suma de penalizaciones") != std::string::npos) {
            break;
        }

        std::stringstream ss(line);
        std::string employee_id;
        std::getline(ss, employee_id, ':');

        int employee_index = -1;
        for (int i = 0; i < num_employees; ++i) {
            if (instance.getStaff(i).ID == employee_id) {
                employee_index = i;
                break;
            }
        }

        if (employee_index == -1) {
            continue;
        }

        std::string segment;
        while (ss >> segment) {
            if (segment.front() == '(' && segment.back() == ')') {
                std::string content = segment.substr(1, segment.length() - 2);
                std::stringstream content_ss(content);
                std::string day_str, shift_id;
                std::getline(content_ss, day_str, ',');
                std::getline(content_ss, shift_id);

                int day = std::stoi(day_str);
                int shift_index = -1;
                for (int i = 0; i < instance.getNumShiftTypes(); ++i) {
                    if (instance.getShift(i).ShiftID == shift_id) {
                        shift_index = i + 1;
                        break;
                    }
                }
                if (shift_index != -1) {
                    schedule.setAssignment(employee_index, day, shift_index);
                }
            }
        }
    }

    return schedule;
}

bool testInstance10Feasibility() {
    Instance instance;
    if (!instance.loadFromFile("nsp_instancias/instances1_24/Instance10.txt")) {
        std::cerr << "Error: Failed to load instance file" << std::endl;
        return false;
    }

    Schedule schedule = parseSolution("instancias_solucion/Instance10.txt", instance);

    ConstraintEvaluator evaluator(instance);
    bool feasible = evaluator.isFeasible(schedule);

    std::cout << "Is solution for Instance 10 feasible? " << (feasible ? "Yes" : "No") << std::endl;
    if (!feasible) {
        std::cout << "Hard constraint violations: " << evaluator.getHardConstraintViolations(schedule) << std::endl;
    }

    return feasible;
}

void registerInstance10ValidatorTests(TestRunner& runner) {
    runner.logTest("Instance 10 Solution Feasibility", testInstance10Feasibility());
}
