#include "neighborhood.h"
#include <cstdlib>
#include <algorithm>

Neighborhood::Neighborhood(int num_employees, int horizon, int num_shift_types)
    : num_employees(num_employees), horizon(horizon), num_shift_types(num_shift_types) {}

Move Neighborhood::getRandomMove(const Schedule& schedule) {
    Move move;
    move.employee_id = rand() % num_employees;
    move.day = rand() % horizon;
    move.old_shift = schedule.getAssignment(move.employee_id, move.day);
    move.new_shift = rand() % (num_shift_types + 1);
    return move;
}

void Neighborhood::perturb(Schedule& schedule, double rate) {
    int assignments_to_perturb = static_cast<int>(num_employees * horizon * rate);
    for (int k = 0; k < assignments_to_perturb; ++k) {
        int emp = rand() % num_employees;
        int day = rand() % horizon;
        int shift = rand() % (num_shift_types + 1);
        schedule.setAssignment(emp, day, shift);
    }
}

Schedule Neighborhood::changeSingleShift(const Schedule& schedule, int num_shift_types) {
    Schedule neighbor = schedule;
    int num_employees = neighbor.getNumEmployees();
    int horizon = neighbor.getHorizonDays();

    int employee = rand() % num_employees;
    int day = rand() % horizon;
    int new_shift = rand() % (num_shift_types + 1);

    neighbor.setAssignment(employee, day, new_shift);
    return neighbor;
}

Schedule Neighborhood::swapShifts(const Schedule& schedule) {
    Schedule neighbor = schedule;
    int num_employees = neighbor.getNumEmployees();
    int horizon = neighbor.getHorizonDays();

    if (num_employees < 2) {
        return neighbor;
    }

    int day = rand() % horizon;
    int employee1 = rand() % num_employees;
    int employee2 = rand() % num_employees;

    while (employee1 == employee2) {
        employee2 = rand() % num_employees;
    }

    int shift1 = neighbor.getAssignment(employee1, day);
    int shift2 = neighbor.getAssignment(employee2, day);

    neighbor.setAssignment(employee1, day, shift2);
    neighbor.setAssignment(employee2, day, shift1);

    return neighbor;
}

Schedule Neighborhood::swapShiftsBlock(const Schedule& schedule) {
    Schedule neighbor = schedule;
    int num_employees = neighbor.getNumEmployees();
    int horizon = neighbor.getHorizonDays();

    if (num_employees < 2) {
        return neighbor;
    }

    int employee1 = rand() % num_employees;
    int employee2 = rand() % num_employees;
    while (employee1 == employee2) {
        employee2 = rand() % num_employees;
    }

    int day1 = rand() % horizon;
    int day2 = rand() % horizon;
    if (day1 > day2) {
        std::swap(day1, day2);
    }

    for (int day = day1; day <= day2; ++day) {
        int shift1 = neighbor.getAssignment(employee1, day);
        int shift2 = neighbor.getAssignment(employee2, day);
        neighbor.setAssignment(employee1, day, shift2);
        neighbor.setAssignment(employee2, day, shift1);
    }

    return neighbor;
}
