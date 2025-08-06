#include "neighborhood.h"
#include "../utils/random.h"
#include <algorithm>
#include <vector>

Neighborhood::Neighborhood(int num_employees, int horizon, int num_shift_types, ConstraintEvaluator& evaluator)
    : num_employees(num_employees), horizon(horizon), num_shift_types(num_shift_types), evaluator(evaluator) {}

Move Neighborhood::getRandomMove(const Schedule& schedule) {
    // Si la solución NO es factible, prioriza los movimientos reparadores
    if (evaluator.getHardConstraintViolations(schedule) < 0) {
        int choice = Random::getInt(0, 99);
        if (choice < 40) { // 40% de probabilidad
            return generateRebalanceWorkloadMove(schedule);
        } else if (choice < 80) { // 40% de probabilidad
            return generateConsolidateWorkMove(schedule);
        }
        // El 20% restante se deja para movimientos de rotación y aleatorios
    }

    // Lógica original para el resto de los casos
    int move_type = Random::getInt(0, 4);
    switch (move_type) {
        case 0: return getRandomChangeMove(schedule);
        case 1: return getRandomSwapMove(schedule);
        case 2: return getRandomBlockSwapMove(schedule);
        case 3: return getRandomRuinAndRecreateMove(schedule);
        case 4: return generateFixShiftRotationMove(schedule);
        default: return getRandomChangeMove(schedule);
    }
}

void Neighborhood::perturb(Schedule& schedule, double rate) {
    int num_moves = static_cast<int>(num_employees * horizon * rate);
    for (int i = 0; i < num_moves; ++i) {
        Move move = getRandomMove(schedule);
        // This is a simplified application, a real implementation would
        // need to handle move application more robustly
        if (move.type == MoveType::Change) {
            schedule.setAssignment(move.employee1, move.day1, move.shift2);
        }
    }
}

Move Neighborhood::getRandomChangeMove(const Schedule& schedule) {
    Move move;
    move.type = MoveType::Change;
    move.employee1 = Random::getInt(0, num_employees - 1);
    move.day1 = Random::getInt(0, horizon - 1);
    move.shift1 = schedule.getAssignment(move.employee1, move.day1);
    move.shift2 = Random::getInt(0, num_shift_types);
    return move;
}

Move Neighborhood::getRandomSwapMove(const Schedule& schedule) {
    Move move;
    move.type = MoveType::Swap;
    move.employee1 = Random::getInt(0, num_employees - 1);
    move.day1 = Random::getInt(0, horizon - 1);
    move.employee2 = Random::getInt(0, num_employees - 1);
    move.day2 = Random::getInt(0, horizon - 1);
    move.shift1 = schedule.getAssignment(move.employee1, move.day1);
    move.shift2 = schedule.getAssignment(move.employee2, move.day2);
    return move;
}

Move Neighborhood::getRandomBlockSwapMove(const Schedule& schedule) {
    Move move;
    move.type = MoveType::BlockSwap;
    move.employee1 = Random::getInt(0, num_employees - 1);
    move.employee2 = Random::getInt(0, num_employees - 1);
    move.day1 = Random::getInt(0, horizon - 2);
    move.day2 = move.day1 + 1;
    return move;
}

Move Neighborhood::getRandomRuinAndRecreateMove(const Schedule& schedule) {
    Move move;
    move.type = MoveType::RuinAndRecreate;
    move.employee1 = Random::getInt(0, num_employees - 1);
    return move;
}

Move Neighborhood::generateFixShiftRotationMove(const Schedule& schedule) {
    // Placeholder implementation
    return getRandomChangeMove(schedule);
}

Move Neighborhood::findFixShiftRotationMove(const Schedule& schedule) {
    // Placeholder implementation
    return getRandomChangeMove(schedule);
}

Move Neighborhood::generateFixHardConstraintMove(const Schedule& schedule) {
    std::vector<std::pair<int, int>> violations = evaluator.getViolatingAssignments(schedule);
    if (violations.empty()) {
        return getRandomChangeMove(schedule); // No hay nada que arreglar, haz un movimiento aleatorio
    }

    // Elige una violación al azar para intentar arreglarla
    std::pair<int, int> assignment_to_fix = violations[Random::getInt(0, violations.size() - 1)];
    int employee = assignment_to_fix.first;
    int day = assignment_to_fix.second;

    Move move;
    move.type = MoveType::Change; // Un simple cambio puede ser un buen reparador
    move.employee1 = employee;
    move.day1 = day;
    move.shift1 = schedule.getAssignment(employee, day);
    
    // Intenta cambiar a un día libre (shift 0), es una buena heurística para arreglar violaciones
    move.shift2 = 0; 
    
    return move;
}

Move Neighborhood::generateBalanceWorkingTimeMove(const Schedule& schedule) {
    // Find an employee violating working time constraints
    for (int emp = 0; emp < num_employees; ++emp) {
        if (evaluator.getEmployeeHardConstraintViolations(schedule, emp) < 0) {
            const Staff& worker = evaluator.instance.getStaff(emp);
            int total_minutes = 0;
            for (int day = 0; day < horizon; ++day) {
                int shift_id = schedule.getAssignment(emp, day);
                if (shift_id > 0) {
                    total_minutes += evaluator.instance.getShift(shift_id - 1).mins;
                }
            }

            Move move;
            move.type = MoveType::Change;
            move.employee1 = emp;

            if (total_minutes < worker.MinTotalMinutes) {
                // Add a shift on a day off, preferably next to an existing shift
                for (int day = 1; day < horizon - 1; ++day) {
                    if (schedule.getAssignment(emp, day) == 0 &&
                        (schedule.getAssignment(emp, day - 1) > 0 || schedule.getAssignment(emp, day + 1) > 0)) {
                        move.day1 = day;
                        move.shift1 = 0;
                        move.shift2 = 1; // Assign first available shift type
                        return move;
                    }
                }
            } else if (total_minutes > worker.MaxTotalMinutes) {
                // Remove a shift, preferably from the end of a block
                for (int day = 1; day < horizon - 1; ++day) {
                    if (schedule.getAssignment(emp, day) > 0 &&
                        (schedule.getAssignment(emp, day - 1) == 0 || schedule.getAssignment(emp, day + 1) == 0)) {
                        move.day1 = day;
                        move.shift1 = schedule.getAssignment(emp, day);
                        move.shift2 = 0;
                        return move;
                    }
                }
            }
        }
    }
    // Fallback if no specific violation is found or no smart move is possible
    return getRandomChangeMove(schedule);
}

Move Neighborhood::generateFixMaxConsecutiveShiftsMove(const Schedule& schedule) {
    // Find an employee violating max consecutive shifts
    for (int emp = 0; emp < num_employees; ++emp) {
        int consecutive_shifts = 0;
        for (int day = 0; day < horizon; ++day) {
            if (schedule.getAssignment(emp, day) > 0) {
                consecutive_shifts++;
            } else {
                consecutive_shifts = 0;
            }

            if (consecutive_shifts > evaluator.instance.getStaff(emp).MaxConsecutiveShifts) {
                Move move;
                move.type = MoveType::Change;
                move.employee1 = emp;
                move.day1 = day;
                move.shift1 = schedule.getAssignment(emp, day);
                move.shift2 = 0;
                return move;
            }
        }
    }
    return getRandomChangeMove(schedule);
}

Move Neighborhood::generateFixMinConsecutiveShiftsMove(const Schedule& schedule) {
    // Find an employee violating min consecutive shifts
    for (int emp = 0; emp < num_employees; ++emp) {
        int consecutive_shifts = 0;
        for (int day = 0; day < horizon; ++day) {
            if (schedule.getAssignment(emp, day) > 0) {
                consecutive_shifts++;
            } else {
                if (consecutive_shifts > 0 && consecutive_shifts < evaluator.instance.getStaff(emp).MinConsecutiveShifts) {
                    Move move;
                    move.type = MoveType::Change;
                    move.employee1 = emp;
                    move.day1 = day;
                    move.shift1 = 0;
                    move.shift2 = 1;
                    return move;
                }
                consecutive_shifts = 0;
            }
        }
    }
    return getRandomChangeMove(schedule);
}

Move Neighborhood::generateFixMinConsecutiveDaysOffMove(const Schedule& schedule) {
    // Find an employee violating min consecutive days off
    for (int emp = 0; emp < num_employees; ++emp) {
        int consecutive_days_off = 0;
        for (int day = 0; day < horizon; ++day) {
            if (schedule.getAssignment(emp, day) == 0) {
                consecutive_days_off++;
            } else {
                if (consecutive_days_off > 0 && consecutive_days_off < evaluator.instance.getStaff(emp).MinConsecutiveDaysOff) {
                    Move move;
                    move.type = MoveType::Change;
                    move.employee1 = emp;
                    move.day1 = day;
                    move.shift1 = schedule.getAssignment(emp, day);
                    move.shift2 = 0;
                    return move;
                }
                consecutive_days_off = 0;
            }
        }
    }
    return getRandomChangeMove(schedule);
}

Move Neighborhood::generateRebalanceWorkloadMove(const Schedule& schedule) {
    std::vector<int> overworked_employees;
    std::vector<int> underworked_employees;

    // 1. Identificar empleados con carga de trabajo incorrecta
    for (int emp = 0; emp < num_employees; ++emp) {
        const Staff& worker = evaluator.instance.getStaff(emp);
        int total_minutes = 0;
        for (int day = 0; day < horizon; ++day) {
            int shift_id = schedule.getAssignment(emp, day);
            if (shift_id > 0) {
                total_minutes += evaluator.instance.getShift(shift_id - 1).mins;
            }
        }

        if (total_minutes > worker.MaxTotalMinutes) {
            overworked_employees.push_back(emp);
        } else if (total_minutes < worker.MinTotalMinutes) {
            underworked_employees.push_back(emp);
        }
    }

    // 2. Si no hay empleados que reequilibrar, devuelve un movimiento aleatorio
    if (overworked_employees.empty() || underworked_employees.empty()) {
        return getRandomChangeMove(schedule);
    }

    // 3. Elige un par de empleados para reequilibrar
    int emp_over = overworked_employees[Random::getInt(0, overworked_employees.size() - 1)];
    int emp_under = underworked_employees[Random::getInt(0, underworked_employees.size() - 1)];

    // 4. Encuentra un día en el que el empleado sobrecargado trabaje y el infrautilizado no
    std::vector<int> possible_days;
    for (int day = 0; day < horizon; ++day) {
        if (schedule.getAssignment(emp_over, day) != 0 && schedule.getAssignment(emp_under, day) == 0) {
            // Adicionalmente, verifica si el empleado infrautilizado puede hacer ese turno
            const Staff& under_worker_info = evaluator.instance.getStaff(emp_under);
            int shift_to_move_id = schedule.getAssignment(emp_over, day);
            if(under_worker_info.MaxShifts[shift_to_move_id - 1] != "0")
            {
                possible_days.push_back(day);
            }
        }
    }

    // 5. Si se encuentra un día, crea el movimiento de transferencia
    if (!possible_days.empty()) {
        int day_to_swap = possible_days[Random::getInt(0, possible_days.size() - 1)];
        int shift_to_move = schedule.getAssignment(emp_over, day_to_swap);
        
        Move move;
        move.type = MoveType::Swap; // Usamos un tipo Swap, pero entre diferentes empleados y días conceptualmente
        move.employee1 = emp_over;
        move.day1 = day_to_swap;
        move.shift1 = shift_to_move; // Turno original del sobrecargado

        move.employee2 = emp_under;
        move.shift2 = 0; // Turno original del infrautilizado (día libre)

        return move;
    }

    // Si no se encontró un día ideal, devuelve un movimiento aleatorio como fallback
    return getRandomChangeMove(schedule);
}

Move Neighborhood::generateConsolidateWorkMove(const Schedule& schedule) {
    for (int emp = 0; emp < num_employees; ++emp) {
        const Staff& worker = evaluator.instance.getStaff(emp);
        // Itera buscando bloques de trabajo demasiado cortos
        for (int day = 0; day < horizon; ++day) {
            if (schedule.getAssignment(emp, day) != 0) {
                int consecutive = 0;
                for (int k = day; k < horizon; ++k) {
                    if (schedule.getAssignment(emp, k) != 0) {
                        consecutive++;
                    } else {
                        break;
                    }
                }

                if (consecutive > 0 && consecutive < worker.MinConsecutiveShifts) {
                    // ¡Encontramos un bloque de trabajo demasiado corto!
                    // Estrategia: Mover el primer día de este bloque (day) a otro lugar.
                    int shift_to_move = schedule.getAssignment(emp, day);

                    // Buscar un día libre para este empleado que esté junto a otro bloque de trabajo
                    for (int target_day = 0; target_day < horizon; ++target_day) {
                        // Si el target_day es un día libre...
                        if (schedule.getAssignment(emp, target_day) == 0) {
                            // Y si el día anterior O el siguiente tienen trabajo...
                            bool adjacent_to_work = false;
                            if (target_day > 0 && schedule.getAssignment(emp, target_day - 1) != 0) adjacent_to_work = true;
                            if (target_day < horizon - 1 && schedule.getAssignment(emp, target_day + 1) != 0) adjacent_to_work = true;

                            if (adjacent_to_work) {
                                // ¡Encontramos un lugar para consolidar!
                                // Creamos un movimiento para intercambiar el día de trabajo aislado con el día libre bien ubicado.
                                Move move;
                                move.type = MoveType::Swap; // Es un swap dentro del mismo empleado en diferentes días
                                move.employee1 = emp;
                                move.day1 = day;
                                move.shift1 = shift_to_move;
                                
                                move.employee2 = emp; // Mismo empleado
                                move.day2 = target_day;
                                move.shift2 = 0; // Día libre

                                return move;
                            }
                        }
                    }
                }
                day += consecutive; // Saltar al final del bloque ya evaluado
            }
        }
    }

    // Si no se encontró nada que consolidar, fallback
    return getRandomChangeMove(schedule);
}
