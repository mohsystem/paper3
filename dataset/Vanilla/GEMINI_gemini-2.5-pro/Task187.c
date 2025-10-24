// This code uses POSIX threads and semaphores.
// Compile with: gcc your_file_name.c -o your_executable -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// A simple reusable barrier implementation
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int trip_count;
    void (*action)(void*);
    void* action_arg;
} Barrier;

void barrier_init(Barrier* b, int count, void (*action)(void*), void* action_arg) {
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
    b->count = 0;
    b->trip_count = count;
    b->action = action;
    b->action_arg = action_arg;
}

void barrier_destroy(Barrier* b) {
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->cond);
}

void barrier_wait(Barrier* b) {
    pthread_mutex_lock(&b->mutex);
    b->count++;
    if (b->count == b->trip_count) {
        if (b->action) {
            b->action(b->action_arg);
        }
        b->count = 0; // Reset for next use
        pthread_cond_broadcast(&b->cond);
    } else {
        pthread_cond_wait(&b->cond, &b->mutex);
    }
    pthread_mutex_unlock(&b->mutex);
}

// H2O context structure
typedef struct {
    sem_t hSem;
    sem_t oSem;
    Barrier barrier;
    char* result_buffer;
    int result_index;
    pthread_mutex_t result_mutex;
} H2O;

void reset_sems(void* arg) {
    H2O* h2o = (H2O*)arg;
    sem_post(&h2o->hSem);
    sem_post(&h2o->hSem);
    sem_post(&h2o->oSem);
}

void releaseHydrogen(H2O* h2o) {
    pthread_mutex_lock(&h2o->result_mutex);
    h2o->result_buffer[h2o->result_index++] = 'H';
    pthread_mutex_unlock(&h2o->result_mutex);
}

void releaseOxygen(H2O* h2o) {
    pthread_mutex_lock(&h2o->result_mutex);
    h2o->result_buffer[h2o->result_index++] = 'O';
    pthread_mutex_unlock(&h2o->result_mutex);
}

void* hydrogen_thread(void* arg) {
    H2O* h2o = (H2O*)arg;
    sem_wait(&h2o->hSem);
    barrier_wait(&h2o->barrier);
    releaseHydrogen(h2o);
    return NULL;
}

void* oxygen_thread(void* arg) {
    H2O* h2o = (H2O*)arg;
    sem_wait(&h2o->oSem);
    barrier_wait(&h2o->barrier);
    releaseOxygen(h2o);
    return NULL;
}

void run_test(const char* input) {
    int len = strlen(input);
    H2O h2o;

    // Initialization
    sem_init(&h2o.hSem, 0, 2);
    sem_init(&h2o.oSem, 0, 1);
    barrier_init(&h2o.barrier, 3, reset_sems, &h2o);
    h2o.result_buffer = (char*)malloc(sizeof(char) * (len + 1));
    h2o.result_index = 0;
    pthread_mutex_init(&h2o.result_mutex, NULL);
    
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * len);

    for (int i = 0; i < len; ++i) {
        if (input[i] == 'H') {
            pthread_create(&threads[i], NULL, hydrogen_thread, &h2o);
        } else if (input[i] == 'O') {
            pthread_create(&threads[i], NULL, oxygen_thread, &h2o);
        }
    }

    for (int i = 0; i < len; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    h2o.result_buffer[h2o.result_index] = '\0';

    printf("Input: %s\n", input);
    printf("Output: %s\n", h2o.result_buffer);
    printf("---\n");

    // Cleanup
    free(threads);
    free(h2o.result_buffer);
    sem_destroy(&h2o.hSem);
    sem_destroy(&h2o.oSem);
    barrier_destroy(&h2o.barrier);
    pthread_mutex_destroy(&h2o.result_mutex);
}

int main() {
    const char* test_cases[] = {
        "HOH",
        "OOHHHH",
        "HHHHHHOOO",
        "HOHOHH",
        "OOOHHHHHH"
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; i++) {
        run_test(test_cases[i]);
    }

    return 0;
}