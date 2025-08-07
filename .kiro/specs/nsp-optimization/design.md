# Design Document

## Overview

El diseño implementa mejoras específicas basadas en investigación académica para el algoritmo de Simulated Annealing aplicado al Nurse Scheduling Problem. Las mejoras principales incluyen: (1) una heurística de generación de solución inicial factible de 5 pasos, (2) implementación de 8 estructuras de vecindario especializadas, (3) evaluación incremental verdadera para eliminar cuellos de botella, y (4) ajuste automático de parámetros basado en el tamaño del problema. Estas mejoras están fundamentadas en el paper de investigación que demuestra su efectividad estadística.

## Architecture

### Modular Structure

```
nsp/
├── src/
│   ├── core/
│   │   ├── data_structures.h/cpp    # Estructuras de datos optimizadas
│   │   ├── instance_parser.h/cpp    # Parsing de instancias
│   │   └── solution.h/cpp           # Representación de soluciones
│   ├── constraints/
│   │   ├── hard_constraints.h/cpp   # Restricciones duras
│   │   ├── soft_constraints.h/cpp   # Restricciones blandas
│   │   ├── constraint_evaluator.h/cpp # Evaluador unificado
│   │   └── incremental_evaluator.h/cpp # Evaluación incremental verdadera
│   ├── metaheuristics/
│   │   ├── simulated_annealing.h/cpp # SA con parámetros adaptativos
│   │   ├── initial_solution.h/cpp    # Heurística de solución inicial de 5 pasos
│   │   ├── neighborhood.h/cpp        # 8 estructuras de vecindario del paper
│   │   └── parameter_tuning.h/cpp    # Ajuste automático de parámetros
│   ├── utils/
│   │   ├── timer.h/cpp              # Medición de tiempo
│   │   ├── random.h/cpp             # Generación aleatoria
│   │   └── logger.h/cpp             # Sistema de logging
│   └── moves/
│       ├── merge_split_move.h/cpp   # Movimientos Merge/Split (NS1, NS2)
│       ├── block_swap_move.h/cpp    # Block Swap (NS3)
│       ├── three_way_swap_move.h/cpp # 3-Way-Swap (NS8)
│       └── combined_neighborhood.h/cpp # Combined Neighborhood Structure (CNS)
├── tests/
│   ├── test_runner.cpp              # Ejecutor de pruebas
│   ├── initial_solution_tests.cpp   # Pruebas de heurística inicial
│   ├── neighborhood_tests.cpp       # Pruebas de movimientos
│   ├── incremental_eval_tests.cpp   # Pruebas de evaluación incremental
│   └── performance_tests.cpp        # Benchmarks comparativos
├── instances/                       # Instancias de prueba
└── main.cpp                         # Punto de entrada principal
```

## Components and Interfaces

### Core Components

#### Initial Solution Generator (Heurística de 5 Pasos)
```cpp
class InitialSolutionGenerator {
private:
    const Instance& instance;
    
public:
    Schedule generateFeasibleSolution();
    
private:
    void assignAnnualLeave(Schedule& schedule);           // Paso 1: Licencias anuales
    void assignWeekends(Schedule& schedule);              // Paso 2: Fines de semana
    void assignInitialDays(Schedule& schedule);           // Paso 3: Primeros 4 días
    void assignRemainingHorizon(Schedule& schedule);      // Paso 4: Resto del horizonte
    void adjustWorkingHours(Schedule& schedule);          // Paso 5: Ajuste de horas
    
    bool canAssignShift(int employee, int day, int shift, const Schedule& schedule) const;
    std::vector<int> getAvailableEmployees(int day, int shift, const Schedule& schedule) const;
};
```

#### Enhanced Neighborhood Structures (8 Movimientos del Paper)
```cpp
class CombinedNeighborhood {
private:
    std::vector<std::unique_ptr<NeighborhoodMove>> moves;
    
public:
    CombinedNeighborhood(const Instance& instance);
    Move getBestMove(const Schedule& schedule, const IncrementalEvaluator& evaluator);
    
private:
    void initializeMoves();
};

// Movimiento Merge/Split (NS1, NS2)
class MergeSplitMove : public NeighborhoodMove {
public:
    Move generateMove(const Schedule& schedule) override;
    
private:
    Move generateMergeMove(const Schedule& schedule);     // M + E -> L
    Move generateSplitMove(const Schedule& schedule);     // L -> M + E
};

// Block Swap (NS3)
class BlockSwapMove : public NeighborhoodMove {
public:
    Move generateMove(const Schedule& schedule) override;
    
private:
    bool isValidBlockSwap(int emp1, int emp2, int day1, int day2, const Schedule& schedule) const;
};

// 3-Way-Swap (NS8)
class ThreeWaySwapMove : public NeighborhoodMove {
public:
    Move generateMove(const Schedule& schedule) override;
    
private:
    bool isValidThreeWaySwap(int emp1, int emp2, int emp3, int day, const Schedule& schedule) const;
};
```

#### True Incremental Evaluator
```cpp
class IncrementalEvaluator {
private:
    const Instance& instance;
    double current_total_score;
    std::vector<double> employee_constraint_scores;  // Score por empleado
    std::vector<double> day_coverage_scores;         // Score por día
    std::vector<double> constraint_contributions;    // Score por tipo de restricción
    
public:
    IncrementalEvaluator(const Instance& instance);
    
    void initialize(const Schedule& schedule);
    double evaluateMove(const Move& move, const Schedule& schedule);
    void applyMove(const Move& move);
    void revertMove(const Move& move);
    
    double getCurrentScore() const { return current_total_score; }
    
private:
    double calculateEmployeeDelta(int employee, const Move& move, const Schedule& schedule);
    double calculateCoverageDelta(const Move& move, const Schedule& schedule);
    void updateEmployeeScore(int employee, double delta);
    void updateCoverageScore(int day, double delta);
};
```

#### Adaptive Parameter Tuning
```cpp
class ParameterTuner {
public:
    struct SAParameters {
        double initial_temperature;
        double cooling_rate;
        int max_iterations;
        int stagnation_limit;
    };
    
    static SAParameters getParametersForProblemSize(int num_employees, int horizon_days);
    
private:
    static SAParameters small_problem_params;   // 1-10 enfermeras
    static SAParameters medium_problem_params;  // 11-30 enfermeras  
    static SAParameters large_problem_params;   // 31-60 enfermeras
};
```

### Research-Based Optimization Strategy

#### Phase 1: Initial Solution Generation (Mayor Impacto)
- Implementar la heurística de 5 pasos del paper para generar soluciones iniciales factibles
- Reemplazar `schedule.randomize()` por `InitialSolutionGenerator::generateFeasibleSolution()`
- Asegurar que el SA comience desde un punto mucho mejor que una solución aleatoria
- Validar que la solución inicial cumple con la mayoría de restricciones duras

#### Phase 2: Enhanced Neighborhood Structures (Corazón del Algoritmo)
- Implementar las 8 estructuras de vecindario del paper
- Priorizar Merge/Split (NS1, NS2), Block Swap (NS3), y 3-Way-Swap (NS8)
- Implementar Combined Neighborhood Structure (CNS) que prueba todos los movimientos
- Reemplazar la selección aleatoria de movimientos por selección inteligente del mejor

#### Phase 3: True Incremental Evaluation (Prerequisito Crítico)
- Refactorizar completamente el IncrementalEvaluator para evaluación verdaderamente incremental
- Calcular solo el delta de restricciones afectadas por cada movimiento
- Mantener estado de puntuaciones por empleado y por día para updates eficientes
- Eliminar el cuello de botella de re-evaluación completa en cada iteración

#### Phase 4: Adaptive Parameter Tuning
- Implementar selección automática de parámetros basada en tamaño del problema
- Usar diferentes configuraciones para problemas pequeños, medianos y grandes
- Basar los parámetros en los valores optimizados reportados en el paper
- Documentar qué conjunto de parámetros se usa para cada instancia

## Data Models

### Move Representation for 8 Neighborhood Structures

```cpp
// Base class for all move types
class Move {
public:
    enum Type { CHANGE, SWAP, MERGE, SPLIT, BLOCK_SWAP, THREE_WAY_SWAP };
    
    virtual ~Move() = default;
    virtual Type getType() const = 0;
    virtual void apply(Schedule& schedule) = 0;
    virtual void revert(Schedule& schedule) = 0;
    virtual std::string toString() const = 0;
};

// Merge Move: M + E -> L (NS1)
class MergeMove : public Move {
private:
    int employee_morning;    // Empleado con turno M
    int employee_evening;    // Empleado con turno E  
    int day;                // Día del merge
    
public:
    MergeMove(int emp_m, int emp_e, int d) : employee_morning(emp_m), employee_evening(emp_e), day(d) {}
    Type getType() const override { return MERGE; }
    void apply(Schedule& schedule) override;
    void revert(Schedule& schedule) override;
};

// Split Move: L -> M + E (NS2)
class SplitMove : public Move {
private:
    int employee_long;       // Empleado con turno L
    int employee_free;       // Empleado libre que tomará E
    int day;                // Día del split
    
public:
    SplitMove(int emp_l, int emp_f, int d) : employee_long(emp_l), employee_free(emp_f), day(d) {}
    Type getType() const override { return SPLIT; }
    void apply(Schedule& schedule) override;
    void revert(Schedule& schedule) override;
};

// Block Swap: Intercambio cruzado (NS3)
class BlockSwapMove : public Move {
private:
    int employee1, employee2;
    int day1, day2;
    int old_shift1_day1, old_shift1_day2;  // Turnos originales de emp1
    int old_shift2_day1, old_shift2_day2;  // Turnos originales de emp2
    
public:
    BlockSwapMove(int e1, int e2, int d1, int d2) : employee1(e1), employee2(e2), day1(d1), day2(d2) {}
    Type getType() const override { return BLOCK_SWAP; }
    void apply(Schedule& schedule) override;
    void revert(Schedule& schedule) override;
};

// 3-Way Swap: Intercambio cíclico (NS8)
class ThreeWaySwapMove : public Move {
private:
    int employee1, employee2, employee3;
    int day;
    int old_shift1, old_shift2, old_shift3;  // Turnos originales
    
public:
    ThreeWaySwapMove(int e1, int e2, int e3, int d) : employee1(e1), employee2(e2), employee3(e3), day(d) {}
    Type getType() const override { return THREE_WAY_SWAP; }
    void apply(Schedule& schedule) override;
    void revert(Schedule& schedule) override;
};
```

### Incremental Evaluation State

```cpp
class IncrementalEvaluationState {
private:
    // Puntuaciones por empleado para restricciones que los afectan individualmente
    struct EmployeeScores {
        double max_shifts_penalty;
        double consecutive_shifts_penalty;
        double min_rest_penalty;
        double weekend_penalty;
        double shift_requests_penalty;
    };
    
    // Puntuaciones por día para restricciones de cobertura
    struct DayScores {
        double coverage_penalty;
        double skill_coverage_penalty;
    };
    
    std::vector<EmployeeScores> employee_scores;
    std::vector<DayScores> day_scores;
    double total_score;
    
public:
    void initialize(const Schedule& schedule, const Instance& instance);
    double calculateMoveDelta(const Move& move, const Schedule& schedule, const Instance& instance);
    void applyMoveDelta(const Move& move, double delta);
    void revertMoveDelta(const Move& move, double delta);
    
    double getTotalScore() const { return total_score; }
};
```

## Error Handling

### Validation Strategy
- Validación de entrada en el parser
- Verificación de consistencia en estructuras de datos
- Detección temprana de violaciones de restricciones duras
- Logging detallado de errores y warnings

### Exception Safety
- Uso de RAII para manejo de recursos
- Strong exception safety en operaciones críticas
- Rollback automático en caso de errores durante la búsqueda

## Testing Strategy

### Unit Testing
- Pruebas para cada componente individual
- Verificación de restricciones por separado
- Pruebas de estructuras de datos optimizadas

### Integration Testing
- Pruebas end-to-end con instancias conocidas
- Verificación de equivalencia con versión original
- Pruebas de regresión automáticas

### Performance Testing
- Benchmarking de cada optimización
- Profiling de memoria y CPU
- Comparación de tiempos de ejecución

### Test Framework
```cpp
class TestRunner {
public:
    void runInstanceTest(const std::string& instance_file);
    void runPerformanceTest(const std::string& instance_file, int iterations);
    void runRegressionTests();
    bool compareResults(const Solution& original, const Solution& optimized);
};
```

## CUDA Integration Strategy

### Parallelizable Components
1. **Constraint Evaluation**: Cada restricción puede evaluarse en paralelo
2. **Neighborhood Generation**: Múltiples movimientos pueden explorarse simultáneamente  
3. **Population-based Methods**: Múltiples soluciones pueden evolucionar en paralelo

### Memory Management
- Uso de unified memory para simplificar transferencias
- Estructuras de datos coalesced para acceso eficiente
- Minimización de transferencias CPU-GPU

### Kernel Design
```cpp
// Ejemplo de kernel para evaluación de restricciones
__global__ void evaluateConstraintsKernel(
    const int* schedule,
    const InstanceData* instance,
    double* constraint_scores,
    int num_employees,
    int horizon_days
);
```

## Performance Targets Based on Research

### Phase 1 (Initial Solution Generation)
- Generar soluciones iniciales factibles o muy cercanas a la factibilidad (>90% de restricciones duras cumplidas)
- Reducir el número de iteraciones necesarias para encontrar la primera solución factible en 80-90%
- Mejorar la puntuación inicial promedio en 50-70% comparado con inicialización aleatoria

### Phase 2 (Enhanced Neighborhood Structures)
- Implementar los 8 movimientos del paper con correctitud verificada
- Demostrar que Combined Neighborhood Structure (CNS) supera estadísticamente a movimientos individuales
- Mejorar la calidad de soluciones finales en 15-25% comparado con movimientos básicos

### Phase 3 (True Incremental Evaluation)
- Acelerar la evaluación de movimientos en 10-50x eliminando re-evaluación completa
- Permitir probar todos los 8 tipos de movimiento en cada iteración sin penalización de rendimiento
- Mantener exactitud perfecta (delta incremental = evaluación completa)

### Phase 4 (Adaptive Parameter Tuning)
- Seleccionar automáticamente parámetros óptimos basados en tamaño del problema
- Mejorar el rendimiento promedio en 10-20% comparado con parámetros fijos
- Resolver exitosamente instancias de diferentes tamaños (1-60 enfermeras) con parámetros apropiados

### Overall System Performance
- Resolver Instance1, Instance2, Instance3, y instancias adicionales con alta calidad
- Reducir tiempo total de ejecución en 60-80% mediante todas las optimizaciones combinadas
- Generar soluciones consistentemente mejores que la implementación original