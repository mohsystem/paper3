#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Note: This code uses POSIX unnamed semaphores.
// It is best compiled on Linux: gcc -o task task.c -pthread
// On macOS, unnamed semaphores are deprecated and this may not work.

typedef struct {
    int n;
    sem_t sem_zero;
    sem_t sem_even;
    sem_t sem_odd;
} ZeroEvenOdd;

void zero(ZeroEvenOdd* obj, void (*printNumber)(int)) {
    for (int i = 1; i <= obj->n; ++i) {
        sem_wait(&obj->sem_zero);
        printNumber(0);
        if (i % 2 == 0) {
            sem_post(&obj->sem_even);
        } else {
            sem_post(&obj->sem_odd);
        }
    }
}

void even(ZeroEvenOdd* obj, void (*printNumber)(int)) {
    for (int i = 2; i <= obj->n; i += 2) {
        sem_wait(&obj->sem_even);
        printNumber(i);
        sem_post(&obj->sem_zero);
    }
}

void odd(ZeroEvenOdd* obj, void (*printNumber)(int)) {
    for (int i = 1; i <= obj->n; i += 2) {
        sem_wait(&obj->sem_odd);
        printNumber(i);
        sem_post(&obj->sem_zero);
    }
}

typedef struct {
    ZeroEvenOdd* obj;
    void (*printNumber)(int);
} ThreadArgs;

void* zero_thread_runner(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    zero(args->obj, args->printNumber);
    return NULL;
}

void* even_thread_runner(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    even(args->obj, args->printNumber);
    return NULL;
}

void* odd_thread_runner(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    odd(args->obj, args->printNumber);
    return NULL;
}

ZeroEvenOdd* zeroEvenOdd_create(int n) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (!obj) return NULL;
    obj->n = n;
    sem_init(&obj->sem_zero, 0, 1);
    sem_init(&obj->sem_even, 0, 0);
    sem_init(&obj->sem_odd, 0, 0);
    return obj;
}

void zeroEvenOdd_destroy(ZeroEvenOdd* obj) {
    if (!obj) return;
    sem_destroy(&obj->sem_zero);
    sem_destroy(&obj->sem_even);
    sem_destroy(&obj->sem_odd);
    free(obj);
}

void print_number(int x) {
    printf("%d", x);
}

void run_test(int n) {
    printf("Testing with n = %d\n", n);
    
    ZeroEvenOdd* zeo = zeroEvenOdd_create(n);
    if (!zeo) {
        perror("Failed to create ZeroEvenOdd object");
        return;
    }

    ThreadArgs args = { .obj = zeo, .printNumber = print_number };
    
    pthread_t threadA, threadB, threadC;
    
    pthread_create(&threadA, NULL, zero_thread_runner, &args);
    pthread_create(&threadB, NULL, even_thread_runner, &args);
    pthread_create(&threadC, NULL, odd_thread_runner, &args);
    
    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadC, NULL);
    
    printf("\n\n");
    
    zeroEvenOdd_destroy(zeo);
}

int main() {
    int test_cases[] = {2, 5, 1, 6, 10};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_cases; ++i) {
        run_test(test_cases[i]);
    }
    
    return 0;
}