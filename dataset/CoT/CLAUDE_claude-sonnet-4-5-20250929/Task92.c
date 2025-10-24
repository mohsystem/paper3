
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int currentCounter;
    int maxCounter;
    pthread_mutex_t lock;
} Task92;

typedef struct {
    Task92* task;
    char threadName[50];
    int iterations;
} ThreadArgs;

void Task92_init(Task92* task, int maxCount) {
    task->currentCounter = 0;
    task->maxCounter = maxCount;
    pthread_mutex_init(&task->lock, NULL);
}

void Task92_destroy(Task92* task) {
    pthread_mutex_destroy(&task->lock);
}

void accessSharedResource(Task92* task, const char* threadName) {
    if (task->currentCounter <= task->maxCounter) {
        pthread_mutex_lock(&task->lock);
        // Double-check after acquiring lock
        if (task->currentCounter <= task->maxCounter) {
            task->currentCounter++;
            printf("%s is accessing currentCounter: %d\\n", threadName, task->currentCounter);
        }
        pthread_mutex_unlock(&task->lock);
    }
}

void* threadFunction(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 0; i < args->iterations; i++) {
        accessSharedResource(args->task, args->threadName);
    }
    return NULL;
}

int main() {
    // Test Case 1: Multiple threads with counter limit 10
    printf("Test Case 1: maxCounter = 10\\n");
    Task92 task1;
    Task92_init(&task1, 10);
    pthread_t threads1[5];
    ThreadArgs args1[5];
    for (int i = 0; i < 5; i++) {
        args1[i].task = &task1;
        snprintf(args1[i].threadName, 50, "Thread-%d", i + 1);
        args1[i].iterations = 3;
        pthread_create(&threads1[i], NULL, threadFunction, &args1[i]);
    }
    for (int i = 0; i < 5; i++) {
        pthread_join(threads1[i], NULL);
    }
    Task92_destroy(&task1);

    // Test Case 2: Counter limit 5
    printf("\\nTest Case 2: maxCounter = 5\\n");
    Task92 task2;
    Task92_init(&task2, 5);
    pthread_t threads2[3];
    ThreadArgs args2[3];
    for (int i = 0; i < 3; i++) {
        args2[i].task = &task2;
        snprintf(args2[i].threadName, 50, "Thread-%d", i + 1);
        args2[i].iterations = 2;
        pthread_create(&threads2[i], NULL, threadFunction, &args2[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads2[i], NULL);
    }
    Task92_destroy(&task2);

    // Test Case 3: Counter limit 0
    printf("\\nTest Case 3: maxCounter = 0\\n");
    Task92 task3;
    Task92_init(&task3, 0);
    pthread_t thread3;
    ThreadArgs args3;
    args3.task = &task3;
    snprintf(args3.threadName, 50, "Thread-1");
    args3.iterations = 1;
    pthread_create(&thread3, NULL, threadFunction, &args3);
    pthread_join(thread3, NULL);
    Task92_destroy(&task3);

    // Test Case 4: Counter limit 20
    printf("\\nTest Case 4: maxCounter = 20\\n");
    Task92 task4;
    Task92_init(&task4, 20);
    pthread_t threads4[10];
    ThreadArgs args4[10];
    for (int i = 0; i < 10; i++) {
        args4[i].task = &task4;
        snprintf(args4[i].threadName, 50, "Thread-%d", i + 1);
        args4[i].iterations = 2;
        pthread_create(&threads4[i], NULL, threadFunction, &args4[i]);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads4[i], NULL);
    }
    Task92_destroy(&task4);

    // Test Case 5: Single thread with counter limit 3
    printf("\\nTest Case 5: Single thread, maxCounter = 3\\n");
    Task92 task5;
    Task92_init(&task5, 3);
    pthread_t thread5;
    ThreadArgs args5;
    args5.task = &task5;
    snprintf(args5.threadName, 50, "Thread-Single");
    args5.iterations = 5;
    pthread_create(&thread5, NULL, threadFunction, &args5);
    pthread_join(thread5, NULL);
    Task92_destroy(&task5);

    return 0;
}
