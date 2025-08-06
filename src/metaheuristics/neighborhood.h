#ifndef NEIGHBORHOOD_H
#define NEIGHBORHOOD_H

#include "../core/data_structures.h"
#include "../core/move.h"

class Neighborhood {
public:
    Neighborhood(int num_employees, int horizon, int num_shift_types);

    Move getRandomMove(const Schedule& schedule);
    void perturb(Schedule& schedule, double rate);

    // Generates a neighbor by changing a single shift
    static Schedule changeSingleShift(const Schedule& schedule, int num_shift_types);

    // Generates a neighbor by swapping shifts between two employees
    static Schedule swapShifts(const Schedule& schedule);

    // Generates a neighbor by swapping a block of shifts
    static Schedule swapShiftsBlock(const Schedule& schedule);

private:
    int num_employees;
    int horizon;
    int num_shift_types;
};

#endif // NEIGHBORHOOD_H
