# 👑 Resolución del Problema de las N-Reinas usando MPI + Pthreads

![C](https://img.shields.io/badge/Language-C-blue.svg)
![MPI](https://img.shields.io/badge/Parallelism-MPI-red.svg)
![Pthreads](https://img.shields.io/badge/Threads-POSIX%20Pthreads-green.svg)
![License](https://img.shields.io/badge/Academic-UNLP-orange.svg)

Solución paralela e híbrida (memoria distribuida + memoria compartida) para resolver el clásico problema de las **N-Reinas**, optimizado mediante operaciones de mapa de bits (*bitboards*) y explotación de simetrías del tablero.

Desarrollado para la asignatura **Sistemas Distribuidos y Paralelos** de la Facultad de Informática / Facultad de Ingeniería de la **Universidad Nacional de La Plata (UNLP)**.

---

## 📌 Aspectos Destacados de la Solución

- **Modelo Híbrido MPI + Pthreads:** 
  - **MPI (Message Passing Interface):** Distribución de subarboles de búsqueda entre múltiples nodos procesadores (memoria distribuida).
  - **Pthreads (POSIX Threads):** Paralelismo multi-hilo a nivel de núcleo/core en cada nodo procesador (memoria compartida).
- **Representación por Bitboards:** Evaluación instantánea de conflictos verticales y diagonales mediante operaciones a nivel de bits (`AND`, `OR`, desplazamientos).
- **Reducción del Espacio de Búsqueda:** Implementación del algoritmo tipo *Takaken* con podado temprano por simetría.
- **Balance de Carga Dinámico:** Estrategia Productor-Consumidor por bloques de trabajo (*chunks*) para evitar inactividad de hilos.

---

## 📂 Estructura del Repositorio

```text
Sistemas-Distribuidos-y-Paralelos/
├── docs/
│   ├── Enunciado-Promocion.pdf   # Consigna oficial de la cátedra
│   └── InformeGrupo5.pdf         # Informe técnico detallado
├── src/
│   ├── nreinas_secuencial.c     # Implementación secuencial de referencia
│   ├── mpi_source.c              # Control MPI, comunicación y temporización
│   ├── pthreads_source.c         # Generación de tareas, gestión de hilos y backtracking
│   └── pthreads_source.h         # Definiciones e interfaces del módulo de hilos
├── scripts/
│   ├── script.sh                 # Script de ejecución individual
│   └── fullScript.sh             # Suite de pruebas automatizada (N=14 a 18)
├── Makefile                      # Automatización de compilación
├── .gitignore                    # Filtros de archivos ejecutables/temporales
└── README.md                     # Documentación general

---

## ⚙️ Compilación y Ejecución


Requisitos Previos
Compilador de C (gcc)

Implementación de MPI (OpenMPI o MPICH)

Librería de hilos POSIX (pthread)

Compilación
Para compilar tanto la versión secuencial como la versión híbrida distribuida:

Bash
make
Para limpiar los ejecutables y archivos objeto generados:

Bash
make clean
Ejecución
1. Versión Secuencial
Bash
./nreinas_secuencial <tamaño_tablero>
Ejemplo:

Bash
./nreinas_secuencial 16
2. Versión Híbrida (MPI + Pthreads)
Bash
mpirun -np <num_procesos_mpi> ./nreinas_paralelo <tamaño_tablero> <hilos_por_proceso>
Ejemplo:

Bash
mpirun -np 2 ./nreinas_paralelo 16 4
👥 Integrantes
Del Blanco, Javier Alberto

Pereira, Armando Antonio

Facultad de Informática / Facultad de Ingeniería — Universidad Nacional de La Plata (UNLP)


---

### 💻 Pasos rápidos para reemplazarlo en Windows (CMD):

1. Abre el bloc de notas desde la consola dentro de tu carpeta `Distri`:
   ```cmd
   notepad README.md