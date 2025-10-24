
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t lock;
} Task92;

typedef struct {
    Task92* task;
    char threadName[50];
} ThreadArgs;

void Task92_init(Task92* task, int maxCounter) {
    task->currentCounter = 0;
    task->maxCounter = maxCounter;
    pthread_mutex_init(&task->lock, NULL);
}

void Task92_destroy(Task92* task) {
    pthread_mutex_destroy(&task->lock);
}

void accessSharedResource(Task92* task, const char* threadName) {
    if (task->currentCounter <= task->maxCounter) {
        pthread_mutex_lock(&task->lock);
        if (task->currentCounter <= task->maxCounter) {
            task->currentCounter++;
            printf("%s is accessing currentCounter: %d\\n", threadName, task->currentCounter);
        }
        pthread_mutex_unlock(&task->lock);
    }
}

void* threadFunction(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    accessSharedResource(args->task, args->threadName);
    free(args);
    return NULL;
}

void createAndRunThreads(Task92* task, int numThreads, const char* prefix) {
    pthread_t* threads = malloc(numThreads * sizeof(pthread_t));
    
    for (int i = 0; i < numThreads; i++) {
        ThreadArgs* args = malloc(sizeof(ThreadArgs));
        args->task = task;
        snprintf(args->threadName, 50, "%s-%d", prefix, i + 1);
        pthread_create(&threads[i], NULL, threadFunction, args);
    }
    
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
}

int main() {
    // Test case 1: Basic test with 5 threads and max counter 10
    printf("Test Case 1:\\n");
    Task92 test1;
    Task92_init(&test1, 10);
    createAndRunThreads(&test1, 5, "Thread");
    Task92_destroy(&test1);
    
    sleep(1);
    
    // Test case 2: Multiple threads with smaller max counter
    printf("\\nTest Case 2:\\n");
    Task92 test2;
    Task92_init(&test2, 3);
    createAndRunThreads(&test2, 5, "Worker");
    Task92_destroy(&test2);
    
    sleep(1);
    
    // Test case 3: Single thread
    printf("\\nTest Case 3:\\n");
    Task92 test3;
    Task92_init(&test3, 1);
    createAndRunThreads(&test3, 1, "SingleThread");
    Task92_destroy(&test3);
    
    sleep(1);
    
    // Test case 4: Max counter is 0
    printf("\\nTest Case 4:\\n");
    Task92 test4;
    Task92_init(&test4, 0);
    createAndRunThreads(&test4, 3, "Thread");
    Task92_destroy(&test4);
    
    sleep(1);
    
    // Test case 5: Many threads with moderate max counter
    printf("\\nTest Case 5:\\n");
    Task92 test5;
    Task92_init(&test5, 7);
    createAndRunThreads(&test5, 10, "Task");
    Task92_destroy(&test5);
    
    return 0;
}
