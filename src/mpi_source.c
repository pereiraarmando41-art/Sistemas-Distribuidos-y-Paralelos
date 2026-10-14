/**************************************************************************/
/* N-Queens Solutions  ver3.1               takaken July/2003             */
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#include "pthreads_source.h"

/* Time in seconds from some point in the past */
double dwalltime();

#define  MAXSIZE  24
#define  MINSIZE   2

// Etiquetas MPI
#define TAG_TASK 0
#define TAG_BACKTRACK1  1
#define TAG_BACKTRACK2  2
#define TAG_RESULT 3
#define TAG_EOF    4
#define TAG_PETICION 5

int  SIZE, SIZEE;
int  MASK, TOPBIT, SIDEMASK;
long int  TOTAL, UNIQUE;

int num_threads; 

/**********************************************/
/* N-Queens Solutions MAIN                    */
/**********************************************/
int main(int argC, char *argV[])
{   
    int rank, nProcs, provided;

    MPI_Init_thread(&argC, &argV, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);

    if (argC < 3) {
        if (rank == 0) {
            printf("Uso: mpirun -np <Nodos> %s <Tablero N> <Hilos T>\n", argV[0]);
            printf("Ejemplo: mpirun -np 4 %s 12 3\n", argV[0]);
        }
        MPI_Finalize();
        return 1;
    }

    num_threads = atoi(argV[2]);

    SIZE=atoi(argV[1]); 

    SIZEE  = SIZE - 1; 
    TOPBIT = 1 << SIZEE; 
    MASK   = (1 << SIZE) - 1; 
    SIDEMASK = TOPBIT | 1;

    double thread_times[32];
    double suma_tiempos_local = 0;
    double total_cluster_time = 0;
    double tIni, tFin, tiempo_total;

    if (rank == 0) {
        double time_root_manager;

        tIni = dwalltime(); 
        task_generator(); 

        root_pthreads_manager(nProcs, num_threads, &UNIQUE, &TOTAL, &time_root_manager, thread_times);

        tFin = dwalltime();
        tiempo_total = tFin - tIni;
        
        suma_tiempos_local = time_root_manager;
        for (int i = 0; i < num_threads - 1; i++) {
            suma_tiempos_local += thread_times[i];
        }

        MPI_Reduce(&suma_tiempos_local, &total_cluster_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        
        printf("\n========================================\n");
        printf("RESULTADOS FINALES\n");
        printf("========================================\n");
        printf("Procesos utilizados: %d | Hilos por proceso: %d\n", nProcs, num_threads);
        printf("Número de resultados: %lu - Únicos: %lu \n", TOTAL, UNIQUE);
        printf("Tiempo Total (Wall-clock): %f segundos \n", tiempo_total);
        
        int total_hilos_sistema = nProcs * num_threads;
        double tiempo_promedio_hilo = total_cluster_time / total_hilos_sistema;
        double balance_carga = tiempo_promedio_hilo / tiempo_total;

        printf("Tiempo Promedio de cómputo por Hilo: %f segundos\n", tiempo_promedio_hilo);
        printf("Balance de Carga (B = T_prom / T_max): %.4f\n", balance_carga);
        printf("========================================\n");

        MPI_Barrier(MPI_COMM_WORLD);
        printf("\n=== TIEMPOS DE HILOS DEL ROOT ===\n");
        printf("Tiempo del Manager ROOT: %.6f segundos\n", time_root_manager);
        for (int i = 0; i < num_threads - 1; i++) {
            printf("Hilo ROOT %d: %.6f segundos\n", i+1, thread_times[i]);
        }
    } else {
        double time_worker_manager;
        
        worker_pthreads_manager(num_threads, &time_worker_manager, thread_times);

        suma_tiempos_local = time_worker_manager;
        for (int i = 0; i < num_threads - 1; i++) {
            suma_tiempos_local += thread_times[i];
        }
        

        MPI_Reduce(&suma_tiempos_local, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);    
        
        MPI_Barrier(MPI_COMM_WORLD);
        printf("\n=== TIEMPOS DE HILOS DEL WORKER %d ===\n", rank);
        printf("Tiempo del Manager WORKER %d: %.6f segundos\n", rank, time_worker_manager);
        for (int i = 0; i < num_threads - 1; i++) {
            printf("Hilo WORKER %d - %d: %.6f segundos\n", rank, i+1, thread_times[i]);
        }
    }
    
    MPI_Finalize();

    return 0;
}

#include <sys/time.h>

double dwalltime()
{
    double sec;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    sec = tv.tv_sec + tv.tv_usec/1000000.0;
    return sec;
}