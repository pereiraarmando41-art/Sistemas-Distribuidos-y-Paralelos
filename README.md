# Resolución del Problema de las N-Reinas usando MPI y Pthreads (Híbrido)

Este repositorio contiene la solución paralela e híbrida (memoria distribuida + memoria compartida) para resolver el problema de las **N-Reinas** optimizado mediante *bitboards* y explotación de simetrías, desarrollado para la asignatura **Sistemas Distribuidos y Paralelos** de la Facultad de Informática / Facultad de Ingeniería de la Universidad Nacional de La Plata (UNLP).

## 📌 Características del Proyecto

- **Modelo de Programación:** Híbrido (**MPI** para comunicación entre nodos + **Pthreads** para paralelismo multi-hilo en cada nodo).
- **Representación:** Operaciones bit a bit (*bitboards*) para verificación instantánea de conflictos diagonales y verticales.
- **Reducción del Espacio de Búsqueda:** Algoritmo optimizado basado en Takaken con podado por simetrías.
- **Balance de Carga:** Modelo Dinámico Máster-Worker / Productor-Consumidor por *chunks* de tareas.

---

## 🛠️ Estructura del Repositorio

- `src/`: Código fuente de la solución secuencial y de la solución híbrida.
  - `nreinas_secuencial.c`: Código base secuencial.
  - `mpi_source.c`: Módulo principal MPI (inicialización, temporización y reducciones).
  - `pthreads_source.c` y `.h`: Generador de tareas, gestor de hilos y backtracking paralelo.
- `scripts/`: Scripts Bash para ejecución en cluster Slurm / PBS.
- `docs/`: Enunciado oficial de la cátedra y el Informe Final en PDF.

---

## 🚀 Compilación y Ejecución

### Requisitos Previos
- Compilador de C (`gcc`)
- Implementación de MPI (`OpenMPI` o `MPICH`)
- Soporte para POSIX Threads (`pthread`)

### Compilación

```bash
make

Ejecución Local / Pruebas
Para ejecutar la versión paralela con N procesadores MPI y T hilos Pthreads por nodo procesando un tablero de tamaño N:

Bash
mpirun -np <num_procesos> ./nreinas_paralelo <tamaño_tablero> <hilos_por_proceso>
Ejemplo:

Bash
mpirun -np 2 ./nreinas_paralelo 16 4

📊 Informe
El archivo Informe-Grupo-10.pdf documenta:

Estrategia de paralelización
Análisis de escalabilidad
Cálculo de speedup, eficiencia y overhead
Conclusiones y comparativa de soluciones


👨‍💻 Integrantes
Del Blanco, Javier Alberto

Pereira, Armando Antonio

Facultad de Informática / Facultad de Ingeniería — Universidad Nacional de La Plata (UNLP)