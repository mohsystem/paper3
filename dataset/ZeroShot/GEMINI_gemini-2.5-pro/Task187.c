#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// Structure to hold shared state for the H2O simulation
typedef struct {
    sem_t h_sem;
    sem_t o_sem;
    pthread_mutex_t mtx;
    int h_atoms_waiting;
    
    // For collecting output safely from multiple threads
    char* output_buffer;
    int output_idx;
    pthread_mutex_t output_mtx;
} H2O;

// Forward declarations of thread functions
void* hydrogen_thread_func(void* arg);
void* oxygen_thread_func(void* arg);

// Initializes the H2O shared state
void h2o_init(H2O* h2o, int capacity) {
    sem_init(&h2o->h_sem, 0, 0);
    sem_init(&h2o->o_sem, 0, 0);
    pthread_mutex_init(&h2o->mtx, NULL);
    pthread_mutex_init(&h2o->output_mtx, NULL);
    h2o->h_atoms_waiting = 0;
    
    h2o->output_buffer = (char*)malloc(sizeof(char) * (capacity + 1));
    if (h2o->output_buffer) {
        h2o->output_buffer[0] = '\0';
    }
    h2o->output_idx = 0;
}

// Cleans up resources used by the H2O shared state
void h2o_destroy(H2O* h2o) {
    sem_destroy(&h2o->h_sem);
    sem_destroy(&h2o->o_sem);
    pthread_mutex_destroy(&h2o->mtx);
    pthread_mutex_destroy(&h2o->output_mtx);
    free(h2o->output_buffer);
}

// Mock releaseHydrogen function
void releaseHydrogen(H2O* h2o) {
    pthread_mutex_lock(&h2o->output_mtx);
    h2o->output_buffer[h2o->output_idx++] = 'H';
    h2o->output_buffer[h2o->output_idx] = '\0';
    pthread_mutex_unlock(&h2o->output_mtx);
}

// Mock releaseOxygen function
void releaseOxygen(H2O* h2o) {
    pthread_mutex_lock(&h2o->output_mtx);
    h2o->output_buffer[h2o->output_idx++] = 'O';
    h2o->output_buffer[h2o->output_idx] = '\0';
    pthread_mutex_unlock(&h2o->output_mtx);
}

// The core logic for hydrogen threads
void hydrogen(H2O* h2o) {
    pthread_mutex_lock(&h2o->mtx);
    h2o->h_atoms_waiting++;
    if (h2o->h_atoms_waiting >= 2) {
        // A pair of H atoms is ready. Wake them up along with an O atom.
        sem_post(&h2o->h_sem);
        sem_post(&h2o->h_sem);
        h2o->h_atoms_waiting -= 2;
        sem_post(&h2o->o_sem);
    }
    pthread_mutex_unlock(&h2o->mtx);
    
    sem_wait(&h2o->h_sem);
    releaseHydrogen(h2o);
}

// The core logic for oxygen threads
void oxygen(H2O* h2o) {
    sem_wait(&h2o->o_sem);
    releaseOxygen(h2o);
}

// Wrapper function for pthread_create
void* hydrogen_thread_func(void* arg) {
    hydrogen((H2O*)arg);
    return NULL;
}

// Wrapper function for pthread_create
void* oxygen_thread_func(void* arg) {
    oxygen((H2O*)arg);
    return NULL;
}

// Helper function to run a single test case
void run_test(const char* input) {
    printf("Testing with input: %s\n", input);
    size_t len = strlen(input);
    
    H2O h2o;
    h2o_init(&h2o, len);

    pthread_t threads[len];
    for (size_t i = 0; i < len; ++i) {
        if (input[i] == 'H') {
            pthread_create(&threads[i], NULL, hydrogen_thread_func, &h2o);
        } else {
            pthread_create(&threads[i], NULL, oxygen_thread_func, &h2o);
        }
    }

    for (size_t i = 0; i < len; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Output: %s\n", h2o.output_buffer);
    printf("--------------------\n");
    
    h2o_destroy(&h2o);
}

int main() {
    const char* test_cases[] = {"HOH", "OOHHHH", "HHHHOO", "HOOHH", "HHHHHHOOO"};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < num_tests; ++i) {
        run_test(test_cases[i]);
    }
    return 0;
}