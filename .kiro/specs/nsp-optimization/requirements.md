# Requirements Document

## Introduction

Este proyecto busca reestructurar y optimizar gradualmente una implementación existente del Nurse Scheduling Problem (NSP) que actualmente funciona solo para la Instance1. El objetivo es mejorar la estructura del código, aplicar optimizaciones básicas, mejorar la metaheurística y eventualmente incorporar CUDA para acelerar el procesamiento. Es fundamental que después de cada tarea se pueda ejecutar una prueba para verificar que el código sigue funcionando correctamente.

## Requirements

### Requirement 1

**User Story:** Como desarrollador, quiero reestructurar el código existente del NSP manteniendo la funcionalidad actual, para que sea más mantenible y escalable.

#### Acceptance Criteria

1. WHEN se reestructure el código THEN el programa SHALL seguir resolviendo correctamente la Instance1
2. WHEN se ejecute una prueba THEN el programa SHALL producir el mismo resultado que la versión original
3. WHEN se organice el código THEN SHALL separarse en módulos lógicos (parsing, estructuras de datos, restricciones, metaheurística)
4. WHEN se refactorice THEN SHALL mantenerse la compatibilidad con el formato de entrada actual

### Requirement 2

**User Story:** Como desarrollador, quiero aplicar optimizaciones básicas a nivel de estructura de datos, para que el programa sea más eficiente en memoria y velocidad.

#### Acceptance Criteria

1. WHEN se optimicen las estructuras de datos THEN SHALL reemplazarse vectores por estructuras más eficientes donde sea apropiado
2. WHEN se apliquen optimizaciones THEN el programa SHALL mantener la funcionalidad existente
3. WHEN se ejecute una prueba THEN el programa SHALL seguir resolviendo la Instance1 correctamente
4. WHEN se mida el rendimiento THEN SHALL mostrarse una mejora medible en tiempo de ejecución o uso de memoria

### Requirement 3

**User Story:** Como desarrollador, quiero implementar una heurística de generación de solución inicial factible basada en investigación, para que el algoritmo SA comience desde un punto mucho mejor que una solución completamente aleatoria.

#### Acceptance Criteria

1. WHEN se implemente la heurística de solución inicial THEN SHALL generar una solución factible o muy cercana a la factibilidad siguiendo el algoritmo de 5 pasos del paper
2. WHEN se genere la solución inicial THEN SHALL asignar primero las licencias anuales (PreAssignedDaysOff)
3. WHEN se asignen los fines de semana THEN SHALL garantizar que cada enfermera tenga al menos 2 fines de semana libres
4. WHEN se procesen los primeros 4 días THEN SHALL considerar las restricciones que dependen del horario anterior
5. WHEN se complete el resto del horizonte THEN SHALL asignar turnos día por día cumpliendo con la cobertura requerida
6. WHEN se ajusten las horas finales THEN SHALL verificar que cada enfermera cumpla con las horas mínimas requeridas

### Requirement 4

**User Story:** Como desarrollador, quiero implementar las 8 estructuras de vecindario del paper de investigación, para que el algoritmo SA tenga movimientos más inteligentes y específicos para el problema NSP.

#### Acceptance Criteria

1. WHEN se implementen los movimientos Merge/Split THEN SHALL convertir turnos M+E en L cuando las preferencias lo justifiquen, y viceversa
2. WHEN se implemente Block Swap THEN SHALL intercambiar turnos entre 2 enfermeras en 2 días diferentes de forma cruzada
3. WHEN se implemente 3-Way-Swap THEN SHALL intercambiar turnos de 3 enfermeras en el mismo día de forma cíclica
4. WHEN se use Combined Neighborhood Structure (CNS) THEN SHALL probar los 8 tipos de movimiento en cada iteración y elegir el mejor
5. WHEN se genere un movimiento THEN SHALL verificar que mantiene la factibilidad antes de aplicarlo
6. WHEN se evalúe un movimiento THEN SHALL usar evaluación incremental para calcular el delta de forma eficiente

### Requirement 5

**User Story:** Como desarrollador, quiero implementar evaluación incremental verdadera para los movimientos, para que el algoritmo pueda probar múltiples movimientos por iteración sin el cuello de botella de re-evaluación completa.

#### Acceptance Criteria

1. WHEN se evalúe un movimiento THEN SHALL calcular solo el delta de las restricciones afectadas por el cambio
2. WHEN se cambie el turno de una enfermera THEN SHALL recalcular únicamente las puntuaciones de restricciones que involucran a esa enfermera
3. WHEN se aplique un movimiento THEN SHALL actualizar incrementalmente el estado del evaluador
4. WHEN se rechace un movimiento THEN SHALL poder revertir los cambios sin recalcular desde cero
5. WHEN se compare con evaluación completa THEN SHALL producir exactamente los mismos resultados
6. WHEN se mida el rendimiento THEN SHALL mostrar una mejora significativa en velocidad de evaluación

### Requirement 6

**User Story:** Como desarrollador, quiero implementar ajuste automático de parámetros de SA basado en el tamaño del problema, para que el algoritmo se adapte automáticamente a diferentes instancias.

#### Acceptance Criteria

1. WHEN se detecte el tamaño del problema THEN SHALL seleccionar automáticamente parámetros apropiados (temperatura inicial, tasa de enfriamiento)
2. WHEN se procesen problemas pequeños (1-10 enfermeras) THEN SHALL usar un conjunto de parámetros optimizado para esa escala
3. WHEN se procesen problemas medianos (11-30 enfermeras) THEN SHALL usar parámetros ajustados para esa escala
4. WHEN se procesen problemas grandes (31-60 enfermeras) THEN SHALL usar parámetros optimizados para esa escala
5. WHEN se ejecute el algoritmo THEN SHALL documentar qué conjunto de parámetros se está usando
6. WHEN se compare con parámetros fijos THEN SHALL mostrar mejor rendimiento promedio en diferentes tamaños de instancia
### R
equirement 7

**User Story:** Como desarrollador, quiero tener un sistema de pruebas automatizado que valide las mejoras implementadas, para que pueda verificar que cada optimización mantiene la correctitud y mejora el rendimiento.

#### Acceptance Criteria

1. WHEN se implemente el sistema de pruebas THEN SHALL poder ejecutar automáticamente pruebas con las instancias disponibles
2. WHEN se ejecute una prueba THEN SHALL verificar que la solución es factible y cumple todas las restricciones duras
3. WHEN se compare resultados THEN SHALL detectar regresiones en la calidad de las soluciones
4. WHEN se mida el rendimiento THEN SHALL comparar tiempos de ejecución antes y después de cada optimización
5. WHEN se valide la heurística inicial THEN SHALL verificar que genera soluciones más factibles que la inicialización aleatoria
6. WHEN se prueben los nuevos movimientos THEN SHALL verificar que mejoran la calidad de las soluciones finales