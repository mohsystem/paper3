#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t mutex;
} SharedResource;

void* worker(void* arg) {
    SharedResource* resource = (SharedResource*)arg;

    while (1) {
        int localValue = -1;
        int shouldBreak = 0;
        
        // Lock the mutex before accessing shared data
        pthread_mutex_lock(&resource->mutex);
        
        // Check-then-act is performed atomically inside the lock to prevent TOCTOU
        if (resource->currentCounter < resource->maxCounter) {
            resource->currentCounter++;
            localValue = resource->currentCounter;
        } else {
            shouldBreak = 1;
        }

        // Unlock the mutex after accessing shared data
        pthread_mutex_unlock(&resource->mutex);

        if (shouldBreak) {
            break;
        }

        if (localValue != -1) {
            // Printing outside the lock to minimize contention
            // pthread_t can be a struct, so we cast to an integer type for printing
            printf("Thread %lu is accessing counter: %d\n", (unsigned long)pthread_self(), localValue);
        }
    }
    return NULL;
}

void run_test(int num_threads, int max_counter) {
    printf("--- Running test with %d threads and max counter %d ---\n", num_threads, max_counter);

    SharedResource resource;
    resource.currentCounter = 0;
    resource.maxCounter = max_counter;

    if (pthread_mutex_init(&resource.mutex, NULL) != 0) {
        perror("Mutex init failed");
        return;
    }

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Failed to allocate memory for threads");
        pthread_mutex_destroy(&resource.mutex);
        return;
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, worker, &resource) != 0) {
            perror("Thread creation failed");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread join failed");
        }
    }

    free(threads);
    pthread_mutex_destroy(&resource.mutex);

    printf("--- Test finished. Final counter should be %d ---\n\n", max_counter);
}

int main() {
    // 5 test cases
    run_test(5, 10);
    run_test(10, 20);
    run_test(2, 50);
    run_test(20, 100);
    run_test(8, 8);

    return 0;
}