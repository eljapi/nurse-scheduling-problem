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

**User Story:** Como desarrollador, quiero mejorar la metaheurística de Simulated Annealing, para que pueda resolver instancias más complejas además de la Instance1.

#### Acceptance Criteria

1. WHEN se mejore la metaheurística THEN el programa SHALL resolver correctamente la Instance1, Instance2 e Instance3
2. WHEN se implemente la nueva metaheurística THEN SHALL mantener la estructura de parámetros configurable
3. WHEN se ejecute una prueba THEN el programa SHALL producir soluciones factibles para múltiples instancias
4. WHEN se compare con la versión anterior THEN SHALL mostrar mejores resultados en términos de calidad de solución

### Requirement 4

**User Story:** Como desarrollador, quiero preparar el código para la integración con CUDA, para que eventualmente pueda aprovechar el procesamiento paralelo en GPU.

#### Acceptance Criteria

1. WHEN se prepare para CUDA THEN el código SHALL organizarse en funciones que puedan ser paralelizadas
2. WHEN se identifiquen secciones paralelizables THEN SHALL documentarse qué partes son candidatas para CUDA
3. WHEN se reestructure THEN SHALL separarse la lógica de cálculo de la lógica de control
4. WHEN se ejecute una prueba THEN el programa SHALL seguir funcionando correctamente en CPU

### Requirement 5

**User Story:** Como desarrollador, quiero tener un sistema de pruebas automatizado, para que pueda verificar que cada cambio mantiene la funcionalidad correcta.

#### Acceptance Criteria

1. WHEN se implemente el sistema de pruebas THEN SHALL poder ejecutar automáticamente pruebas con las instancias disponibles
2. WHEN se ejecute una prueba THEN SHALL verificar que la solución es factible
3. WHEN se compare resultados THEN SHALL detectar regresiones en la calidad de las soluciones
4. WHEN se ejecuten las pruebas THEN SHALL completarse en un tiempo razonable (< 30 segundos por instancia)

### Requirement 6

**User Story:** Como desarrollador, quiero documentar el código y las optimizaciones aplicadas, para que sea fácil entender y mantener el proyecto.

#### Acceptance Criteria

1. WHEN se documente el código THEN SHALL incluir comentarios explicativos en las funciones principales
2. WHEN se apliquen optimizaciones THEN SHALL documentarse el razonamiento detrás de cada cambio
3. WHEN se reestructure THEN SHALL actualizarse la documentación del proyecto
4. WHEN se prepare para CUDA THEN SHALL documentarse la estrategia de paralelización