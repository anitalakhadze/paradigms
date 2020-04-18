#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARR_SIZE 100
#define NUM_WORKERS 5
#define NUM_BRICKS ARR_SIZE
void *transfer_ice_creams (void *ptr);

pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;

typedef enum {false, true} bool; 

int from_arr[ARR_SIZE];
int to_arr[ARR_SIZE];
int counter = NUM_BRICKS - 1;

typedef struct worker_t{
    int id;
    pthread_t thread;
} worker_t; 

int main(){
    worker_t *workers = malloc(NUM_WORKERS * sizeof(worker_t)); 
    for (int i = 0; i < ARR_SIZE; i++){
        from_arr[i] = i; 
    }
    for (int i = 0; i < NUM_WORKERS; i++){
        workers[i].id = i + 1;
        pthread_create(&workers[i].thread, NULL, transfer_ice_creams, workers + i);
    }
    for (int i = 0; i < NUM_WORKERS; i++){
        pthread_join(workers[i].thread, NULL);        
    }
    for (int i = 0; i < ARR_SIZE; i++){
        printf("value is %i \n", to_arr[i]);
    }
    free(workers);
    return 0;
}

void *transfer_ice_creams (void *ptr){
    int my_index = counter;
    worker_t *w = (worker_t *)ptr;
    while (true){
        pthread_mutex_lock(&condition_mutex);
        if (counter < 0) break;
        else {
         my_index = counter;
         counter--;
        }
        pthread_mutex_unlock(&condition_mutex);
        printf("counter now is %i, but my_index is %i \n", counter, my_index);
        to_arr[my_index] = from_arr[my_index];
    }
    pthread_mutex_unlock(&condition_mutex);
}