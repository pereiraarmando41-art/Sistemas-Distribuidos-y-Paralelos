#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#include "pthreads_source.h"

// Etiquetas MPI
#define TAG_TASK 0
#define TAG_BACKTRACK1  1
#define TAG_BACKTRACK2  2
#define TAG_RESULT 3
#define TAG_EOF    4
#define TAG_PETICION 5

//Para el root, esta sera la lista de tareas generadas por el, para los workers esta sera la lista de tareas recibidas en el chunk enviado por el root 
Task task_queue[MAX_QUEUE_SIZE];
int mpi_eof = 0;
int queue_size = 0;      
int next_task_id = 0;     
int tasks_consumed = 0;
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;

long int THREAD_COUNT8 = 0;
long int THREAD_COUNT4 = 0;
long int THREAD_COUNT2 = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;   
pthread_cond_t manager_cond = PTHREAD_COND_INITIALIZER;

// Array para almacenar tiempos de los hilos localmente
static double local_thread_times[32];

/**********************************************/
/* Check Unique Solutions                     */
/**********************************************/
void Check(int *BOARD, int *BOARDE, int *BOARD1, int *BOARD2, long int *COUNT8, long int *COUNT4, long int *COUNT2, int ENDBIT)
{
    int  *own, *you, bit, ptn;

    /* 90-degree rotation */
    if (*BOARD2 == 1) {
        for (ptn=2,own=BOARD+1; own<=BOARDE; own++,ptn<<=1) {
            bit = 1;
            for (you=BOARDE; *you!=ptn && *own>=bit; you--) bit <<= 1;
            if (*own > bit) return;
            if (*own < bit) break;
        }
        if (own > BOARDE) {
            (*COUNT2)++;
            return;
        }
    }

    /* 180-degree rotation */
    if (*BOARDE == ENDBIT) {
        for (you=BOARDE-1,own=BOARD+1; own<=BOARDE; own++,you--) {
            bit = 1;
            for (ptn=TOPBIT; ptn!=*you && *own>=bit; ptn>>=1) bit <<= 1;
            if (*own > bit) return;
            if (*own < bit) break;
        }
        if (own > BOARDE) {
            (*COUNT4)++;
            return;
        }
    }

    /* 270-degree rotation */
    if (*BOARD1 == TOPBIT) {
        for (ptn=TOPBIT>>1,own=BOARD+1; own<=BOARDE; own++,ptn>>=1) {
            bit = 1;
            for (you=BOARD; *you!=ptn && *own>=bit; you++) bit <<= 1;
            if (*own > bit) return;
            if (*own < bit) break;
        }
    }
    (*COUNT8)++;
}
/**********************************************/
/* First queen is inside                      */
/**********************************************/
void Backtrack2(int y, int left, int down, int right, int *BOARD, int *BOARDE, int *BOARD1, int *BOARD2, int BOUND1, int BOUND2, int ENDBIT, int LASTMASK, long int *COUNT8, long int *COUNT4, long int *COUNT2)
{   
    int  bitmap, bit;

    bitmap = MASK & ~(left | down | right);
    if (y == SIZEE) {
        if (bitmap) {
            if (!(bitmap & LASTMASK)) {
                BOARD[y] = bitmap;
                Check(BOARD, BOARDE, BOARD1, BOARD2, COUNT8, COUNT4, COUNT2, ENDBIT);
            }
        }
    } else {
        if (y < BOUND1) {
            bitmap |= SIDEMASK;
            bitmap ^= SIDEMASK;
        } else if (y == BOUND2) {
            if (!(down & SIDEMASK)) return;
            if ((down & SIDEMASK) != SIDEMASK) bitmap &= SIDEMASK;
        }
        while (bitmap) {
            bitmap ^= BOARD[y] = bit = -bitmap & bitmap;
            Backtrack2(y+1, (left | bit)<<1, down | bit, (right | bit)>>1, BOARD, BOARDE, BOARD1, BOARD2, BOUND1, BOUND2, ENDBIT, LASTMASK, COUNT8, COUNT4, COUNT2);
        }
    }
}

/**********************************************/
/* First queen is in the corner               */
/**********************************************/
void Backtrack1(int y, int left, int down, int right, int *BOARD, int BOUND1, long int *COUNT8)
{
    int  bitmap, bit;

    bitmap = MASK & ~(left | down | right); //lugares seguros donde poner una reina
    if (y == SIZEE) {
        if (bitmap) {
            BOARD[y] = bitmap;
            (*COUNT8)++;
        }
    } else {
        if (y < BOUND1) {
            bitmap |= 2;
            bitmap ^= 2;
        }
       while (bitmap) {
            bitmap ^= BOARD[y] = bit = -bitmap & bitmap;
            Backtrack1(y+1, (left | bit)<<1, down | bit, (right | bit)>>1, BOARD, BOUND1, COUNT8);
        }
    }
}


/*******************************************************/
/* Función generadora de tareas utilizada por el root  */
/*******************************************************/
void task_generator(void){
    queue_size = 0; 
    int BOUND1, BOUND2;
    int ENDBIT, LASTMASK;
    
    // Tareas para Backtrack1
    for (BOUND1 = 2; BOUND1 < SIZEE; BOUND1++) {
        int bit1 = 1 << BOUND1;
        int left1 = (2 | bit1) << 1;
        int down1 = 1 | bit1;
        int right1 = bit1 >> 1;

        int bitmap1 = MASK & ~(left1 | down1 | right1);
        if (2 < BOUND1) { bitmap1 |= 2; bitmap1 ^= 2; }

        while (bitmap1) {
            int bit2 = -bitmap1 & bitmap1;
            bitmap1 ^= bit2;
            
            int left2 = (left1 | bit2) << 1;
            int down2 = down1 | bit2;
            int right2 = (right1 | bit2) >> 1;
            
            int bitmap2 = MASK & ~(left2 | down2 | right2);
            if (3 < BOUND1) { bitmap2 |= 2; bitmap2 ^= 2; }

            while (bitmap2) {
                int bit3 = -bitmap2 & bitmap2;
                bitmap2 ^= bit3;

                task_queue[queue_size].type = TAG_BACKTRACK1;
                task_queue[queue_size].bound1 = BOUND1;
                task_queue[queue_size].y = 4;
                task_queue[queue_size].left = (left2 | bit3) << 1;
                task_queue[queue_size].down = down2 | bit3;
                task_queue[queue_size].right = (right2 | bit3) >> 1;
                task_queue[queue_size].board_state[0] = 1;
                task_queue[queue_size].board_state[1] = bit1;
                task_queue[queue_size].board_state[2] = bit2;
                task_queue[queue_size].board_state[3] = bit3;
                queue_size++;
            }
        }
    }

    // Tareas para Backtrack2
    LASTMASK = TOPBIT | 1;
    ENDBIT = TOPBIT >> 1;
    for (BOUND1 = 1, BOUND2 = SIZE - 2; BOUND1 < BOUND2; BOUND1++, BOUND2--) {
        int bit0 = 1 << BOUND1;
        int left0 = bit0 << 1;
        int down0 = bit0;
        int right0 = bit0 >> 1;
        
        int bitmap0 = MASK & ~(left0 | down0 | right0);
        
        if (1 < BOUND1) {
            bitmap0 |= SIDEMASK; bitmap0 ^= SIDEMASK;
        } else if (1 == BOUND2) {
            if (!(down0 & SIDEMASK)) {
                // REEMPLAZO DEL CONTINUE: Forzamos bitmap0 a 0 para que no entre al while(bitmap0)
                bitmap0 = 0; 
            } else if ((down0 & SIDEMASK) != SIDEMASK) {
                bitmap0 &= SIDEMASK;
            }
        }

        while (bitmap0) {
            int bit1 = -bitmap0 & bitmap0;
            bitmap0 ^= bit1;
            
            int left1 = (left0 | bit1) << 1;
            int down1 = down0 | bit1;
            int right1 = (right0 | bit1) >> 1;

            int bitmap1 = MASK & ~(left1 | down1 | right1);
            if (2 < BOUND1) {
                bitmap1 |= SIDEMASK; bitmap1 ^= SIDEMASK;
            } else if (2 == BOUND2) {
                if (!(down1 & SIDEMASK)) {
                    // REEMPLAZO DEL CONTINUE: Forzamos bitmap1 a 0 para que no entre al while(bitmap1)
                    bitmap1 = 0;
                } else if ((down1 & SIDEMASK) != SIDEMASK) {
                    bitmap1 &= SIDEMASK;
                }
            }

            while (bitmap1) {
                int bit2 = -bitmap1 & bitmap1;
                bitmap1 ^= bit2;

                task_queue[queue_size].type = TAG_BACKTRACK2;
                task_queue[queue_size].bound1 = BOUND1;
                task_queue[queue_size].bound2 = BOUND2;
                task_queue[queue_size].endbit = ENDBIT;
                task_queue[queue_size].lastmask = LASTMASK;
                task_queue[queue_size].y = 3;
                task_queue[queue_size].left = (left1 | bit2) << 1;
                task_queue[queue_size].down = down1 | bit2;
                task_queue[queue_size].right = (right1 | bit2) >> 1;
                task_queue[queue_size].board_state[0] = bit0;
                task_queue[queue_size].board_state[1] = bit1;
                task_queue[queue_size].board_state[2] = bit2; 
                queue_size++;
            }
        }

        LASTMASK |= LASTMASK >> 1 | LASTMASK << 1;
        ENDBIT >>= 1;
    }
}

/******************************************************************/
/* Función de los hilos del root que realizan trabajo             */
/******************************************************************/
void* f_thread_root(void *arg) {
    int thread_id = *(int*)arg;
    
    long int COUNT8 = 0, COUNT4 = 0, COUNT2 = 0;
    int BOARD[MAXSIZE];
    int *BOARDE = &BOARD[SIZEE];
    int end = 0;
    Task current_task;

    double t_start = dwalltime();
    while (!end) {
        int bit, tarea = 0;
        
        pthread_mutex_lock(&task_mutex);
 
        if (next_task_id < queue_size) {
            current_task = task_queue[next_task_id++]; 
            tarea = 1;
        } else {
            end = 1;
        }

        pthread_mutex_unlock(&task_mutex);

        if (tarea) {
            if (current_task.type == TAG_BACKTRACK1) {
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
                BOARD[3] = current_task.board_state[3];
                
                Backtrack1(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, current_task.bound1, &COUNT8);
            } 
            else if (current_task.type == TAG_BACKTRACK2) {
                int *BOARD1 = &BOARD[current_task.bound1];
                int *BOARD2 = &BOARD[current_task.bound2];
                
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
            
                Backtrack2(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, BOARDE, BOARD1, BOARD2, 
                           current_task.bound1, current_task.bound2, 
                           current_task.endbit, current_task.lastmask,
                           &COUNT8, &COUNT4, &COUNT2);
            }
        }
    }

    pthread_mutex_lock(&count_mutex);
    THREAD_COUNT8 += COUNT8;
    THREAD_COUNT4 += COUNT4;
    THREAD_COUNT2 += COUNT2;
    pthread_mutex_unlock(&count_mutex);

    double t_end = dwalltime();
    local_thread_times[thread_id] = t_end - t_start;

    pthread_exit(NULL);
}



/******************************************************************/
/* Función de los hilos del worker que realizan trabajo           */
/******************************************************************/
void* f_thread_worker(void *arg) {
    int thread_id = *(int*)arg;
    
    long int COUNT8 = 0, COUNT4 = 0, COUNT2 = 0;
    int BOARD[MAXSIZE];
    int *BOARDE = &BOARD[SIZEE];
    int end = 0;
    Task current_task;

    double t_start = dwalltime();
    while (!end) {
        int bit, tarea = 0;
        
        pthread_mutex_lock(&task_mutex);
        
        while (tasks_consumed == queue_size && !mpi_eof) {
            pthread_cond_signal(&manager_cond); 
            pthread_cond_wait(&task_cond, &task_mutex);
        }

        if(tasks_consumed < queue_size) {
            current_task = task_queue[tasks_consumed++];
            tarea = 1;
        }
        
        if (tasks_consumed == queue_size && mpi_eof) {
            end = 1;
        }

        pthread_mutex_unlock(&task_mutex);

        if (tarea) {
            if (current_task.type == TAG_BACKTRACK1) {
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
                BOARD[3] = current_task.board_state[3];
                
                Backtrack1(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, current_task.bound1, &COUNT8);
            } 
            else if (current_task.type == TAG_BACKTRACK2) {
                int *BOARD1 = &BOARD[current_task.bound1];
                int *BOARD2 = &BOARD[current_task.bound2];
                
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
            
                Backtrack2(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, BOARDE, BOARD1, BOARD2, 
                           current_task.bound1, current_task.bound2, 
                           current_task.endbit, current_task.lastmask,
                           &COUNT8, &COUNT4, &COUNT2);
            }
        }

    }

    pthread_mutex_lock(&count_mutex);
    THREAD_COUNT8 +=  COUNT8;
    THREAD_COUNT4 +=  COUNT4;
    THREAD_COUNT2 +=  COUNT2;
    pthread_mutex_unlock(&count_mutex);

    double t_end = dwalltime();
    local_thread_times[thread_id] = t_end - t_start;
    
    pthread_exit(NULL);
}

/*************************************************************************************************/
/* Tarea del hilo principal del root: administrar el envio de chunks y la recepcion de resultados*/
/*************************************************************************************************/
void root_main_thread(int nProcs) {
    int active_workers = nProcs - 1;
    int flag;
    MPI_Status status;
    MPI_Request requests[nProcs];

    for (int i = 0; i < nProcs; i++) {
        requests[i] = MPI_REQUEST_NULL;
    }

    Task worker_chunks[nProcs][CHUNK_SIZE]; 

    long int local8 = 0, local4 = 0, local2 = 0;
    int BOARD[MAXSIZE];
    int *BOARDE = &BOARD[SIZEE];    

    while (active_workers > 0 || next_task_id < queue_size) {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

        if (flag) {
            int worker_id = status.MPI_SOURCE;
    
            if (status.MPI_TAG == TAG_PETICION) {
                
                MPI_Recv(NULL, 0, MPI_INT, worker_id, TAG_PETICION, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (requests[worker_id] != MPI_REQUEST_NULL) {
                    MPI_Wait(&requests[worker_id], MPI_STATUS_IGNORE);
                } 

                pthread_mutex_lock(&task_mutex);

                if (next_task_id < queue_size) {
                    int tasks_in_chunk = 0;
                    
                    while (next_task_id < queue_size && tasks_in_chunk < CHUNK_SIZE) {
                        worker_chunks[worker_id][tasks_in_chunk++] = task_queue[next_task_id++];
                    }
                    
                    MPI_Isend(worker_chunks[worker_id], tasks_in_chunk * sizeof(Task), MPI_BYTE, worker_id, TAG_TASK, MPI_COMM_WORLD, &requests[worker_id]);
                
                }else {
                    
                    MPI_Send(NULL, 0, MPI_BYTE, worker_id, TAG_EOF, MPI_COMM_WORLD);
                
                }

                pthread_mutex_unlock(&task_mutex); 

            }else if(status.MPI_TAG == TAG_RESULT){
                MPI_Recv(NULL, 0, MPI_LONG, worker_id, TAG_RESULT, MPI_COMM_WORLD, &status);
                active_workers--;
            }
        }

        Task current_task;
        int tiene_tarea = 0;

        pthread_mutex_lock(&task_mutex);
        if (next_task_id < queue_size) {
            current_task = task_queue[next_task_id++];
            tiene_tarea = 1;
        }
        pthread_mutex_unlock(&task_mutex);

        if (tiene_tarea) {
            if (current_task.type == TAG_BACKTRACK1) {
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
                BOARD[3] = current_task.board_state[3];
                
                Backtrack1(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, current_task.bound1, &local8);
            } 
            else if (current_task.type == TAG_BACKTRACK2) {
                int *BOARD1 = &BOARD[current_task.bound1];
                int *BOARD2 = &BOARD[current_task.bound2];
                
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
            
                Backtrack2(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, BOARDE, BOARD1, BOARD2, 
                           current_task.bound1, current_task.bound2, 
                           current_task.endbit, current_task.lastmask,
                           &local8, &local4, &local2);
            }
        }
    }

    for (int i = 0; i < nProcs; i++) {
        if (requests[i] != MPI_REQUEST_NULL) {
            MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
        }
    }
    
    pthread_mutex_lock(&count_mutex);
    THREAD_COUNT8 += local8;
    THREAD_COUNT4 += local4;
    THREAD_COUNT2 += local2;
    pthread_mutex_unlock(&count_mutex);

    pthread_mutex_lock(&task_mutex);
    mpi_eof = 1; 
    pthread_cond_broadcast(&task_cond);
    pthread_mutex_unlock(&task_mutex);
}

/***************************************************************************************************/
/* Tarea del hilo principal del worker: solicitar chunks, recibirlos y colocarlos en la cola de tareas */
/***************************************************************************************************/
void worker_main_thread(void) {
    Task chunk[CHUNK_SIZE];
    MPI_Status status;
    int flag;
    int request_pending = 0;

    long int local8 = 0, local4 = 0, local2 = 0;
    int BOARD[MAXSIZE];
    int *BOARDE = &BOARD[SIZEE];

    while(!mpi_eof) {
        MPI_Iprobe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

        if(flag){
            MPI_Recv(chunk, CHUNK_SIZE * sizeof(Task), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            request_pending = 0;

            pthread_mutex_lock(&task_mutex);
            if(status.MPI_TAG == TAG_EOF){
                mpi_eof = 1;
            } else {
                int received_bytes;
                MPI_Get_count(&status, MPI_BYTE, &received_bytes);
                int num_tasks = received_bytes / sizeof(Task);

                tasks_consumed = 0;
                queue_size = 0;

                for (int i = 0; i < num_tasks; i++) {
                        task_queue[queue_size++] = chunk[i];
                    }
            }

            pthread_cond_broadcast(&task_cond);
            pthread_mutex_unlock(&task_mutex);
        }

        Task current_task;
        int tiene_tarea = 0;

        pthread_mutex_lock(&task_mutex);
        if (tasks_consumed < queue_size){
            current_task = task_queue[tasks_consumed++];
            tiene_tarea = 1;
        }
        else if (!request_pending && !mpi_eof){
            MPI_Send(NULL, 0, MPI_INT, 0, TAG_PETICION, MPI_COMM_WORLD);
            request_pending = 1;
        }
        pthread_mutex_unlock(&task_mutex);

        if (tiene_tarea) {
            if (current_task.type == TAG_BACKTRACK1) {
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
                BOARD[3] = current_task.board_state[3];
                
                Backtrack1(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, current_task.bound1, &local8);
            } 
            else if (current_task.type == TAG_BACKTRACK2) {
                int *BOARD1 = &BOARD[current_task.bound1];
                int *BOARD2 = &BOARD[current_task.bound2];
                
                BOARD[0] = current_task.board_state[0];
                BOARD[1] = current_task.board_state[1];
                BOARD[2] = current_task.board_state[2];
            
                Backtrack2(current_task.y, current_task.left, current_task.down, current_task.right, 
                           BOARD, BOARDE, BOARD1, BOARD2, 
                           current_task.bound1, current_task.bound2, 
                           current_task.endbit, current_task.lastmask,
                           &local8, &local4, &local2);
            }
        }
    }

    pthread_mutex_lock(&count_mutex);
    THREAD_COUNT8 += local8;
    THREAD_COUNT4 += local4;
    THREAD_COUNT2 += local2;
    pthread_mutex_unlock(&count_mutex);
}

/******************************************************************/
/* Función de creación de hilos del proceso root                  */
/******************************************************************/
void root_pthreads_manager(int nProcs, int num_threads, long int *UNIQUE, long int *TOTAL, double *time_root_manager, double *thread_times){

    pthread_t misHilos[num_threads - 1];
    int thread_ids[num_threads - 1];

    THREAD_COUNT8 = 0; THREAD_COUNT4 = 0; THREAD_COUNT2 = 0;

    for (int id = 0; id < num_threads - 1; id++) {
        thread_ids[id] = id;
        pthread_create(&misHilos[id], NULL, f_thread_root, &thread_ids[id]);
    }    
    
    double t_start = dwalltime();
    
    root_main_thread(nProcs);
    
    double t_end = dwalltime();
    *time_root_manager = t_end - t_start;
 
    for (int i = 0; i < num_threads - 1; i++) {
        pthread_join(misHilos[i], NULL);
    }
    
    for (int i = 0; i < num_threads - 1; i++) {
        thread_times[i] = local_thread_times[i];
    }
    
    long int global[3];
    long int local_counts[3] = {THREAD_COUNT8, THREAD_COUNT4, THREAD_COUNT2};
    
    MPI_Reduce(local_counts, global, 3, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    *UNIQUE = global[0] + global[1] + global[2];
    *TOTAL  = global[0] * 8 + global[1] * 4 + global[2] * 2;
}

/******************************************************************/
/* Función de creación de hilos de los procesos worker            */
/******************************************************************/
void worker_pthreads_manager(int num_threads, double *time_worker_manager, double *thread_times) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    pthread_t misHilos[num_threads - 1];
    int thread_ids[num_threads - 1];

    THREAD_COUNT8 = 0; THREAD_COUNT4 = 0; THREAD_COUNT2 = 0;
    queue_size = 0; tasks_consumed = 0; mpi_eof= 0;

    for (int id = 0; id < num_threads - 1; id++) {
        thread_ids[id] = id;
        pthread_create(&misHilos[id], NULL, f_thread_worker, &thread_ids[id]);
    }

    double t_start = dwalltime();
    worker_main_thread();
    double t_end = dwalltime();
    *time_worker_manager = t_end - t_start;
 
    for (int i = 0; i < num_threads - 1; i++) {
        pthread_join(misHilos[i], NULL);
    }
    
    for (int i = 0; i < num_threads - 1; i++) {
        thread_times[i] = local_thread_times[i];
    }

    MPI_Send(NULL, 0, MPI_LONG, 0, TAG_RESULT, MPI_COMM_WORLD);

    long int local_counts[3] = {THREAD_COUNT8, THREAD_COUNT4, THREAD_COUNT2};
    MPI_Reduce(local_counts, NULL, 3, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
}