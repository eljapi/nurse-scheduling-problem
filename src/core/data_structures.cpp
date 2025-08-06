#include "data_structures.h"
#include <random>
#include <chrono>
#include <iostream>

Schedule::Schedule(int employees, int days) 
    : num_employees(employees), horizon_days(days) {
    assignments.resize(employees);
    for (int i = 0; i < employees; i++) {
        assignments[i].resize(days, 0);  // Initialize with 0 (no shift)
    }
}

Schedule::Schedule(const Schedule& other) 
    : num_employees(other.num_employees), horizon_days(other.horizon_days) {
    assignments = other.assignments;
}

Schedule& Schedule::operator=(const Schedule& other) {
    if (this != &other) {
        num_employees = other.num_employees;
        horizon_days = other.horizon_days;
        assignments = other.assignments;
    }
    return *this;
}

void Schedule::setAssignment(int employee, int day, int shift) {
    if (employee >= 0 && employee < num_employees && 
        day >= 0 && day < horizon_days) {
        assignments[employee][day] = shift;
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
}