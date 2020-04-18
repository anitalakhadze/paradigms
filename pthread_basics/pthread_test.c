#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

typedef enum {false, true} bool; 
int num_active_workers = NUM_THREADS;
bool last_thread_finished_job = false; 

pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;

void *arr_func( void *ptr );
int counter = 0; 
int arr[100]; 

typedef struct worker_t{
    pthread_t thread;
    int id;
    int low;
    int high;
} worker_t;

int main()
{
    worker_t *workers = malloc(NUM_THREADS * sizeof(worker_t));   
    for (int i = 0; i < NUM_THREADS; i++){
        workers[i].id = i;
        workers[i].low = i * 100/NUM_THREADS; 
        workers[i].high = workers[i].low + 100/NUM_THREADS;
        pthread_create(&workers[i].thread, NULL, arr_func, workers + i);        
    }
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(workers[i].thread, NULL);        
    }
    free(workers);
    printf("The sum of array %i\n", counter); 
    exit(0); 
}

void *arr_func( void *ptr )
{
    worker_t *w = (worker_t *)ptr;
    printf("Worker of id %i, of range from %i to %i, sends his regards.\n", w->id, w->low, w->high);
    for (int i = w->low; i < w->high; i++){
        arr[i] = w->id;
        //printf("value is %i\n", arr[i]);
    }
    pthread_mutex_lock(&condition_mutex);
    if (num_active_workers > 1){
        num_active_workers--;
        printf("Thread %i is going to sleep because num_active_workers is %i\n", w->id, num_active_workers);
        while (last_thread_finished_job == false){
            pthread_cond_wait(&condition_cond, &condition_mutex);
        }
        printf("Thread %i is free\n", w->id);
    } else {
        printf("Thread %i is chosen to sum all\n", w->id);
        for (int i = 0; i < 100; i++){
            counter += arr[i];
        }
        last_thread_finished_job = true;
        pthread_cond_broadcast(&condition_cond);
    }
    pthread_mutex_unlock(&condition_mutex);
}