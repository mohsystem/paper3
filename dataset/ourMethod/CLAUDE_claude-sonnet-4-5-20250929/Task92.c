
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>

// Global shared resources with proper initialization
static pthread_mutex_t counterMutex = PTHREAD_MUTEX_INITIALIZER;
static int currentCounter = 0;
static const int maxCounter = 20;

// Thread argument structure to pass thread ID safely
typedef struct {
    int threadId;
} ThreadArgs;

// Thread-safe function to manage access to shared resource
// Uses mutex to protect check-then-act sequence against TOCTOU
void* accessSharedResource(void* arg) {
    // Input validation: check for NULL pointer
    if (arg == NULL) {
        return NULL;
    }
    
    ThreadArgs* args = (ThreadArgs*)arg;
    int threadId = args->threadId;
    
    // Validate threadId to prevent negative values
    if (threadId < 0) {
        return NULL;
    }
    
    // Acquire lock before checking to prevent TOCTOU race condition
    // Check return value of pthread_mutex_lock for errors
    if (pthread_mutex_lock(&counterMutex) != 0) {
        fprintf(stderr, "Failed to acquire lock\\n");
        return NULL;
    }
    
    // Check if we can increment (prevent overflow and respect maxCounter)
    // Both check and increment are atomic under the same lock
    if (currentCounter <= maxCounter) {
        // Safe increment with overflow check
        if (currentCounter < INT_MAX) {
            currentCounter++;
            
            // Thread-safe printf: format string is constant, no user-controlled format
            // All output done while holding lock to prevent interleaving
            printf("Thread %d accessing currentCounter: %d\\n", threadId, currentCounter);
            fflush(stdout); // Ensure output is written immediately
        }
    }
    
    // Release lock - check return value for errors
    if (pthread_mutex_unlock(&counterMutex) != 0) {
        fprintf(stderr, "Failed to release lock\\n");
    }
    
    return NULL;
}

// Helper function to create and join threads safely
int runThreadTest(int numThreads) {
    // Input validation: prevent negative or excessive thread counts
    if (numThreads <= 0 || numThreads > 1000) {
        fprintf(stderr, "Invalid thread count\\n");
        return -1;
    }
    
    // Allocate memory for thread handles with size validation
    pthread_t* threads = (pthread_t*)calloc(numThreads, sizeof(pthread_t));
    if (threads == NULL) {
        fprintf(stderr, "Failed to allocate thread array\\n");
        return -1;
    }
    
    // Allocate memory for thread arguments with size validation
    ThreadArgs* args = (ThreadArgs*)calloc(numThreads, sizeof(ThreadArgs));
    if (args == NULL) {
        fprintf(stderr, "Failed to allocate args array\\n");
        free(threads);
        return -1;
    }
    
    // Create threads with proper error checking
    for (int i = 0; i < numThreads; i++) {
        args[i].threadId = i + 1;
        if (pthread_create(&threads[i], NULL, accessSharedResource, &args[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\\n", i);
            // Clean up already created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(args);
            return -1;
        }
    }
    
    // Join all threads with error checking
    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Failed to join thread %d\\n", i);
        }
    }
    
    // Free allocated memory exactly once
    free(threads);
    threads = NULL;
    free(args);
    args = NULL;
    
    return 0;
}

int main(void) {
    // Test case 1: Single thread access
    printf("Test 1: Single thread\\n");
    currentCounter = 0;
    if (runThreadTest(1) != 0) {
        fprintf(stderr, "Test 1 failed\\n");
    }
    
    // Test case 2: Two threads
    printf("\\nTest 2: Two threads\\n");
    currentCounter = 0;
    if (runThreadTest(2) != 0) {
        fprintf(stderr, "Test 2 failed\\n");
    }
    
    // Test case 3: Five threads
    printf("\\nTest 3: Five threads\\n");
    currentCounter = 0;
    if (runThreadTest(5) != 0) {
        fprintf(stderr, "Test 3 failed\\n");
    }
    
    // Test case 4: Ten threads with counter near maxCounter
    printf("\\nTest 4: Ten threads near limit\\n");
    currentCounter = 15;
    if (runThreadTest(10) != 0) {
        fprintf(stderr, "Test 4 failed\\n");
    }
    
    // Test case 5: Threads when counter exceeds maxCounter
    printf("\\nTest 5: Threads when counter exceeds max\\n");
    currentCounter = 21;
    if (runThreadTest(3) != 0) {
        fprintf(stderr, "Test 5 failed\\n");
    }
    
    // Clean up mutex resources
    pthread_mutex_destroy(&counterMutex);
    
    return 0;
}
