#ifndef PTHREADS_SOURCE_H
#define PTHREADS_SOURCE_H

#include <pthread.h>
#include <mpi.h>

#define MAXSIZE 24
#define CHUNK_SIZE 256
#define MAX_QUEUE_SIZE 64000

typedef struct {
    int type;
    int bound1;
    int bound2;
    int endbit;
    int lastmask;
    int y;
    int left;
    int down;
    int right;
    int board_state[4]; // Ahora guardamos hasta 4 posiciones
} Task;

extern int SIZE, SIZEE, MASK, TOPBIT, SIDEMASK;
extern void task_generator(void);
extern void worker_pthreads_manager(int num_threads, double *time_worker_manager, double *thread_times);
extern void root_pthreads_manager(int nProcs, int num_threads, long int *UNIQUE, long int *TOTAL, double *time_root_manager, double *thread_times);
extern double dwalltime(void);

#endif