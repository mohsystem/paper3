#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    sem_t hSem;
    sem_t oSem;
    
    pthread_mutex_t barrierMutex;
    pthread_cond_t barrierCv;
    int barrierCount;
    int generation;

    char* result_str;
    pthread_mutex_t result_mutex;
    int result_idx;
} H2O;

void barrier_wait_and_reset(H2O* h2o) {
    pthread_mutex_lock(&h2o->barrierMutex);
    int my_gen = h2o->generation;
    h2o->barrierCount++;

    if (h2o->barrierCount == 3) {
        h2o->generation++;
        h2o->barrierCount = 0;
        // Barrier action: reset semaphores
        sem_post(&h2o->hSem);
        sem_post(&h2o->hSem);
        sem_post(&h2o->oSem);
        pthread_cond_broadcast(&h2o->barrierCv);
    } else {
        while (my_gen == h2o->generation) {
            pthread_cond_wait(&h2o->barrierCv, &h2o->barrierMutex);
        }
    }
    pthread_mutex_unlock(&h2o->barrierMutex);
}


void releaseHydrogen(H2O* h2o) {
    pthread_mutex_lock(&h2o->result_mutex);
    h2o->result_str[h2o->result_idx++] = 'H';
    pthread_mutex_unlock(&h2o->result_mutex);
}

void releaseOxygen(H2O* h2o) {
    pthread_mutex_lock(&h2o->result_mutex);
    h2o->result_str[h2o->result_idx++] = 'O';
    pthread_mutex_unlock(&h2o->result_mutex);
}

void* hydrogen_thread_func(void* arg) {
    H2O* h2o = (H2O*)arg;
    sem_wait(&h2o->hSem);
    barrier_wait_and_reset(h2o);
    releaseHydrogen(h2o);
    return NULL;
}

void* oxygen_thread_func(void* arg) {
    H2O* h2o = (H2O*)arg;
    sem_wait(&h2o->oSem);
    barrier_wait_and_reset(h2o);
    releaseOxygen(h2o);
    return NULL;
}


void run_simulation(const char* water) {
    int n = strlen(water);
    H2O h2o;

    sem_init(&h2o.hSem, 0, 2);
    sem_init(&h2o.oSem, 0, 1);
    
    pthread_mutex_init(&h2o.barrierMutex, NULL);
    pthread_cond_init(&h2o.barrierCv, NULL);
    h2o.barrierCount = 0;
    h2o.generation = 0;
    
    h2o.result_str = (char*)malloc(sizeof(char) * (n + 1));
    h2o.result_str[n] = '\0';
    h2o.result_idx = 0;
    pthread_mutex_init(&h2o.result_mutex, NULL);
    
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * n);

    for (int i = 0; i < n; ++i) {
        if (water[i] == 'H') {
            pthread_create(&threads[i], NULL, hydrogen_thread_func, &h2o);
        } else { // 'O'
            pthread_create(&threads[i], NULL, oxygen_thread_func, &h2o);
        }
    }

    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Input: %s\n", water);
    printf("Output: %s\n\n", h2o.result_str);
    
    sem_destroy(&h2o.hSem);
    sem_destroy(&h2o.oSem);
    pthread_mutex_destroy(&h2o.barrierMutex);
    pthread_cond_destroy(&h2o.barrierCv);
    pthread_mutex_destroy(&h2o.result_mutex);
    free(h2o.result_str);
    free(threads);
}


int main() {
    const char* testCases[] = {"HOH", "OOHHHH", "HHO", "HHHHHHOOO", "OHHOHH"};
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for(int i = 0; i < num_tests; ++i) {
        run_simulation(testCases[i]);
    }

    return 0;
}