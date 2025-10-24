#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For gettid on some systems if needed, but pthread_self is standard

// Struct to hold shared data and the mutex
typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t mutex;
} SharedResource;

// The function that each thread will execute
void* worker(void* arg) {
    SharedResource* resource = (SharedResource*)arg;

    while (1) {
        int should_break = 0;

        // Acquire lock
        pthread_mutex_lock(&resource->mutex);

        if (resource->currentCounter < resource->maxCounter) {
            resource->currentCounter++;
            // pthread_self() returns an opaque type, casting to unsigned long is common for printing
            printf("Thread %lu is accessing the counter: %d\n", (unsigned long)pthread_self(), resource->currentCounter);
        } else {
            should_break = 1;
        }

        // Release lock
        pthread_mutex_unlock(&resource->mutex);

        if (should_break) {
            break;
        }
    }
    return NULL;
}

int main() {
    const int NUM_THREADS = 5;
    const int MAX_COUNT = 50;

    pthread_t threads[NUM_THREADS];
    SharedResource sharedResource;
    
    sharedResource.currentCounter = 0;
    sharedResource.maxCounter = MAX_COUNT;

    // Initialize the mutex
    if (pthread_mutex_init(&sharedResource.mutex, NULL) != 0) {
        perror("Mutex init failed");
        return 1;
    }

    printf("Starting %d threads to count up to %d\n", NUM_THREADS, MAX_COUNT);

    // Create 5 threads (test cases)
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker, &sharedResource) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread join failed");
            return 1;
        }
    }

    // Destroy the mutex to free resources
    pthread_mutex_destroy(&sharedResource.mutex);

    printf("All threads have finished execution.\n");

    return 0;
}
// To compile and run this C code:
// gcc your_file_name.c -o your_program_name -lpthread
// ./your_program_name