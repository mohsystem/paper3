#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// Define PTHREAD_BARRIER_T for environments that don't have it (like macOS)
#if defined(__APPLE__)
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t *barrier, const void *attr, unsigned int count) {
    if (count == 0) return -1; // EINVAL
    pthread_mutex_init(&barrier->mutex, 0);
    pthread_cond_init(&barrier->cond, 0);
    barrier->trip_count = count;
    barrier->count = 0;
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    ++(barrier->count);
    if (barrier->count >= barrier->trip_count) {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    } else {
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
        pthread_mutex_unlock(&barrier->mutex);
        return 0;
    }
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    return 0;
}
#endif

// Structure to hold shared data for threads
typedef struct {
    sem_t h_sem;
    sem_t o_sem;
    pthread_barrier_t barrier;
    pthread_mutex_t result_mutex;
    char* result_str;
    int* result_idx;
} H2O;

void reset_semaphores(H2O* h2o) {
    sem_post(&h2o->h_sem);
    sem_post(&h2o->h_sem);
    sem_post(&h2o->o_sem);
}

void* hydrogen(void* arg) {
    H2O* h2o = (H2O*)arg;

    sem_wait(&h2o->h_sem);

    // releaseHydrogen()
    pthread_mutex_lock(&h2o->result_mutex);
    h2o->result_str[(*h2o->result_idx)++] = 'H';
    pthread_mutex_unlock(&h2o->result_mutex);

    int barrier_ret = pthread_barrier_wait(&h2o->barrier);
    if (barrier_ret == PTHREAD_BARRIER_SERIAL_THREAD) {
        reset_semaphores(h2o);
    }
    
    return NULL;
}

void* oxygen(void* arg) {
    H2O* h2o = (H2O*)arg;

    sem_wait(&h2o->o_sem);
    
    // releaseOxygen()
    pthread_mutex_lock(&h2o->result_mutex);
    h2o->result_str[(*h2o->result_idx)++] = 'O';
    pthread_mutex_unlock(&h2o->result_mutex);

    int barrier_ret = pthread_barrier_wait(&h2o->barrier);
    if (barrier_ret == PTHREAD_BARRIER_SERIAL_THREAD) {
        reset_semaphores(h2o);
    }

    return NULL;
}

char* runWaterSimulation(const char* water) {
    int len = strlen(water);
    pthread_t* threads = (pthread_t*)malloc(len * sizeof(pthread_t));
    H2O h2o;

    char* result_str = (char*)malloc((len + 1) * sizeof(char));
    int result_idx = 0;
    
    h2o.result_str = result_str;
    h2o.result_idx = &result_idx;

    // Initialize synchronization primitives
    sem_init(&h2o.h_sem, 0, 2);
    sem_init(&h2o.o_sem, 0, 1);
    pthread_barrier_init(&h2o.barrier, NULL, 3);
    pthread_mutex_init(&h2o.result_mutex, NULL);
    
    int thread_count = 0;
    for (int i = 0; i < len; i++) {
        if (water[i] == 'H') {
            pthread_create(&threads[thread_count++], NULL, hydrogen, &h2o);
        } else if (water[i] == 'O') {
            pthread_create(&threads[thread_count++], NULL, oxygen, &h2o);
        }
    }

    for (int i = 0; i < len; i++) {
        pthread_join(threads[i], NULL);
    }

    result_str[len] = '\0';

    // Cleanup
    sem_destroy(&h2o.h_sem);
    sem_destroy(&h2o.o_sem);
    pthread_barrier_destroy(&h2o.barrier);
    pthread_mutex_destroy(&h2o.result_mutex);
    free(threads);

    return result_str;
}

int main() {
    const char* testCases[] = {
        "HOH",
        "OOHHHH",
        "HHOHHO",
        "HHHHHHOOO",
        "OHHOHHOHH"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("Input: %s\n", testCases[i]);
        char* output = runWaterSimulation(testCases[i]);
        printf("Output: %s\n", output);
        printf("--------------------\n");
        free(output);
    }

    return 0;
}