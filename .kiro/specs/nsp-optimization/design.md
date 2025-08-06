# Design Document

## Overview

El diseño se enfoca en una reestructuración incremental del código NSP existente, aplicando optimizaciones progresivas mientras se mantiene la funcionalidad. La arquitectura propuesta separa claramente las responsabilidades y prepara el código para futuras optimizaciones con CUDA.

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
│   │   └── constraint_evaluator.h/cpp # Evaluador unificado
│   ├── metaheuristics/
│   │   ├── simulated_annealing.h/cpp # SA mejorado
│   │   ├── neighborhood.h/cpp        # Operadores de vecindario
│   │   └── cooling_schedule.h/cpp    # Esquemas de enfriamiento
│   ├── utils/
│   │   ├── timer.h/cpp              # Medición de tiempo
│   │   ├── random.h/cpp             # Generación aleatoria
│   │   └── logger.h/cpp             # Sistema de logging
│   └── cuda/
│       ├── cuda_constraints.cu      # Restricciones en CUDA (futuro)
│       └── cuda_utils.cuh           # Utilidades CUDA (futuro)
├── tests/
│   ├── test_runner.cpp              # Ejecutor de pruebas
│   ├── instance_tests.cpp           # Pruebas por instancia
│   └── performance_tests.cpp        # Pruebas de rendimiento
├── instances/                       # Instancias de prueba
└── main.cpp                         # Punto de entrada principal
```

## Components and Interfaces

### Core Components

#### DataStructures
```cpp
class Schedule {
private:
    std::vector<std::vector<int>> assignments; // [employee][day] = shift_id
    int num_employees;
    int horizon_days;
    
public:
    void setAssignment(int employee, int day, int shift);
    int getAssignment(int employee, int day) const;
    Schedule copy() const;
    void randomize(const Instance& instance);
};

class Instance {
private:
    int horizon;
    std::vector<Staff> staff;
    std::vector<Shift> shifts;
    std::vector<DayOff> days_off;
    std::vector<ShiftRequest> shift_requests;
    std::vector<CoverRequirement> cover_requirements;
    
public:
    // Getters optimizados con referencias const
    const std::vector<Staff>& getStaff() const;
    const Shift& getShift(int id) const;
    bool isValidAssignment(int employee, int day, int shift) const;
};
```

#### ConstraintEvaluator
```cpp
class ConstraintEvaluator {
private:
    const Instance& instance;
    HardConstraints hard_constraints;
    SoftConstraints soft_constraints;
    
public:
    double evaluateSchedule(const Schedule& schedule);
    bool isFeasible(const Schedule& schedule);
    double getHardConstraintViolations(const Schedule& schedule);
    double getSoftConstraintPenalties(const Schedule& schedule);
};
```

### Optimization Strategy

#### Phase 1: Code Restructuring
- Separar el código monolítico en módulos
- Reemplazar arrays dinámicos por std::vector con reserva de memoria
- Implementar clases RAII para manejo de memoria
- Crear interfaces claras entre componentes

#### Phase 2: Data Structure Optimization
- Usar std::array para datos de tamaño fijo conocido
- Implementar cache-friendly data layouts
- Optimizar acceso a memoria con localidad espacial
- Usar bitsets para flags booleanos cuando sea apropiado

#### Phase 3: Algorithm Optimization
- Implementar evaluación incremental de restricciones
- Optimizar operadores de vecindario
- Mejorar el esquema de enfriamiento de SA
- Implementar múltiples estrategias de búsqueda local

#### Phase 4: CUDA Preparation
- Identificar funciones paralelizables
- Separar lógica de datos
- Implementar estructuras de datos GPU-friendly
- Crear interfaces para kernels CUDA

## Data Models

### Optimized Data Structures

```cpp
// Reemplazo de vectores por estructuras más eficientes
struct OptimizedStaff {
    std::string id;
    std::array<int, MAX_SHIFT_TYPES> max_shifts;
    int max_total_minutes;
    int min_total_minutes;
    int max_consecutive_shifts;
    int min_consecutive_shifts;
    int min_consecutive_days_off;
    int max_weekends;
};

// Cache-friendly schedule representation
class CompactSchedule {
private:
    std::vector<uint8_t> assignments; // Packed representation
    int num_employees;
    int horizon_days;
    
public:
    void setAssignment(int employee, int day, uint8_t shift);
    uint8_t getAssignment(int employee, int day) const;
    size_t getMemoryFootprint() const;
};
```

### Constraint Evaluation Optimization

```cpp
class IncrementalEvaluator {
private:
    double current_score;
    std::vector<double> constraint_contributions;
    
public:
    double evaluateMove(const Schedule& schedule, int employee, int day, int old_shift, int new_shift);
    void applyMove(int employee, int day, int old_shift, int new_shift);
    void recomputeFromScratch(const Schedule& schedule);
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

## Performance Targets

### Phase 1 (Restructuring)
- Mantener tiempo de ejecución actual (±5%)
- Reducir uso de memoria en 10-20%
- Mejorar legibilidad y mantenibilidad del código

### Phase 2 (Data Structure Optimization)  
- Reducir tiempo de ejecución en 20-30%
- Reducir uso de memoria en 30-40%
- Mantener exactitud de resultados

### Phase 3 (Algorithm Optimization)
- Resolver Instance2 e Instance3 exitosamente
- Mejorar calidad de soluciones en 15-25%
- Reducir tiempo de convergencia en 40-50%

### Phase 4 (CUDA Integration)
- Acelerar evaluación de restricciones 5-10x
- Permitir exploración de vecindarios más grandes
- Escalar a instancias de mayor tamaño