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
  - **MPI (Message Passing Interface):** Distribución de subárboles de búsqueda entre múltiples nodos procesadores (memoria distribuida).
  - **Pthreads (POSIX Threads):** Paralelismo multi-hilo a nivel de núcleo/core en cada nodo procesador (memoria compartida).
- **Representación por Bitboards:** Evaluación instantánea de conflictos verticales y diagonales mediante operaciones a nivel de bits (`AND`, `OR`, desplazamientos).
- **Reducción del Espacio de Búsqueda:** Implementación del algoritmo tipo *Takaken* con podado temprano por simetrías.
- **Balance de Carga Dinámico:** Estrategia Productor-Consumidor por bloques de trabajo (*chunks*) para evitar inactividad de hilos.

---

## 📂 Estructura del Repositorio

- `docs/`: Consigna oficial (`Enunciado-Promocion.pdf`) e informe técnico (`InformeGrupo5.pdf`).
- `src/`: Código fuente de la solución (`nreinas_secuencial.c`, `mpi_source.c`, `pthreads_source.c`, `pthreads_source.h`).
- `scripts/`: Scripts Bash para ejecución en cluster (`script.sh`, `fullScript.sh`).
- `Makefile`: Script para automatización de la compilación.
- `.gitignore`: Filtro de binarios y temporales.

---

## ⚙️ Compilación y Ejecución

### Requisitos Previos

- Compilador de C (`gcc`)
- Implementación de MPI (`OpenMPI` o `MPICH`)
- Librería de hilos POSIX (`pthread`)

### Compilación

Para compilar tanto la versión secuencial como la versión híbrida distribuida:

```bash
make
```

Para limpiar los ejecutables y archivos objeto generados:

```bash
make clean
```

### Ejecución

#### 1. Versión Secuencial
```bash
./nreinas_secuencial <tamaño_tablero>
```

*Ejemplo:*
```bash
./nreinas_secuencial 16
```

#### 2. Versión Híbrida (MPI + Pthreads)
```bash
mpirun -np <num_procesos_mpi> ./nreinas_paralelo <tamaño_tablero> <hilos_por_proceso>
```

*Ejemplo:*
```bash
mpirun -np 2 ./nreinas_paralelo 16 4
```

---

## 📊 Informe

El archivo InformeGrupo5.pdf documenta:

 - Estrategia de paralelización
 - Análisis de escalabilidad
 - Cálculo de speedup, eficiencia y overhead
 - Conclusiones y comparativa de soluciones

---

## 👥 Integrantes

- **Del Blanco, Javier Alberto**
- **Pereira, Armando Antonio**

*Facultad de Informática / Facultad de Ingeniería — Universidad Nacional de La Plata (UNLP)*