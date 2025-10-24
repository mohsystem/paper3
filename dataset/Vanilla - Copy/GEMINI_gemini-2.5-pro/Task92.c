#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Shared resources
int currentCounter;
int maxCounter;
pthread_mutex_t counter_mutex;

/**
 * The function executed by each thread. It increments the shared counter
 * in a thread-safe manner until it reaches the maxCounter.
 */
void* incrementCounter(void* arg) {
    (void)arg; // Unused parameter
    while (1) {
        // Acquire lock
        pthread_mutex_lock(&counter_mutex);

        // Check if the counter is less than the max value
        if (currentCounter < maxCounter) {
            currentCounter++;
            printf("Thread %lu is accessing counter: %d\n", (unsigned long)pthread_self(), currentCounter);
            // Release lock and continue loop
            pthread_mutex_unlock(&counter_mutex);
        } else {
            // Max count reached, release lock and break the loop
            pthread_mutex_unlock(&counter_mutex);
            break;
        }
    }
    return NULL;
}

/**
 * Sets up and runs a single simulation with a given number of threads and a max counter value.
 * @param numThreads The number of threads to create.
 * @param maxVal The maximum value for the counter.
 */
void runSimulation(int numThreads, int maxVal) {
    printf("--- Starting Test: %d threads, maxCounter = %d ---\n", numThreads, maxVal);

    // Reset shared state for the test
    currentCounter = 0;
    maxCounter = maxVal;
    
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Failed to allocate memory for threads");
        return;
    }

    // Create and start each thread
    for (int i = 0; i < numThreads; ++i) {
        if (pthread_create(&threads[i], NULL, incrementCounter, NULL) != 0) {
            perror("Failed to create thread");
        }
    }

    // Wait for all threads to complete their execution
    for (int i = 0; i < numThreads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    
    free(threads);

    printf("--- Test Finished. Final counter value: %d ---\n\n", currentCounter);
}

int main() {
    // Initialize the mutex once for the entire program
    if (pthread_mutex_init(&counter_mutex, NULL) != 0) {
        perror("Mutex init failed");
        return 1;
    }

    // 5 test cases as required
    runSimulation(2, 10);
    runSimulation(5, 20);
    runSimulation(10, 100);
    runSimulation(3, 5);
    runSimulation(8, 50);

    // Destroy the mutex before exiting
    pthread_mutex_destroy(&counter_mutex);
    
    return 0;
}