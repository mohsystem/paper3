#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For usleep

// Struct to hold shared data
typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t lock;
} SharedData;

// The function to be executed by each thread
void* accessResource(void* arg) {
    SharedData* data = (SharedData*)arg;

    while (1) {
        // Acquire lock
        pthread_mutex_lock(&data->lock);

        if (data->currentCounter < data->maxCounter) {
            data->currentCounter++;
            // Using pthread_self() to get a unique thread identifier
            printf("Thread %lu is accessing counter: %d\n", (unsigned long)pthread_self(), data->currentCounter);
            
            // Release lock
            pthread_mutex_unlock(&data->lock);

            // Small sleep to allow other threads to run, making interleaving more visible
            usleep(10000); // 10ms
        } else {
            // Release lock and exit loop if max is reached
            pthread_mutex_unlock(&data->lock);
            break;
        }
    }
    return NULL;
}

void run_test_case(int num_threads, int max_counter) {
    printf("\n--- Test Case: %d threads, max_counter %d ---\n", num_threads, max_counter);

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Failed to allocate memory for threads");
        return;
    }
    
    SharedData data;
    data.currentCounter = 0;
    data.maxCounter = max_counter;

    // Initialize the mutex
    if (pthread_mutex_init(&data.lock, NULL) != 0) {
        printf("Mutex init failed\n");
        free(threads);
        return;
    }

    // Create and start threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, accessResource, &data) != 0) {
            perror("Failed to create thread");
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&data.lock);
    free(threads);

    printf("All threads have finished. Final counter value: %d\n", data.currentCounter);
}

int main() {
    // 5 test cases
    run_test_case(5, 20);
    run_test_case(3, 10);
    run_test_case(8, 50);
    run_test_case(2, 5);
    run_test_case(10, 10);
    
    return 0;
}