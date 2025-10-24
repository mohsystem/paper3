#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int n;
    volatile int turn; // 0 for zero, 1 for odd, 2 for even
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} ZeroEvenOdd;

void zero(ZeroEvenOdd* obj, void (*printNumber)(int)) {
    for (int i = 1; i <= obj->n; ++i) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->turn != 0) {
            pthread_cond_wait(&obj->cond, &obj->mtx);
        }
        printNumber(0);
        if (i % 2 != 0) {
            obj->turn = 1;
        } else {
            obj->turn = 2;
        }
        pthread_cond_broadcast(&obj->cond);
        pthread_mutex_unlock(&obj->mtx);
    }
}

void even(ZeroEvenOdd* obj, void (*printNumber)(int)) {
    for (int i = 2; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->turn != 2) {
            pthread_cond_wait(&obj->cond, &obj->mtx);
        }
        printNumber(i);
        obj->turn = 0;
        pthread_cond_broadcast(&obj->cond);
        pthread_mutex_unlock(&obj->mtx);
    }
}

void odd(ZeroEvenOdd* obj, void (*printNumber)(int)) {
    for (int i = 1; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->turn != 1) {
            pthread_cond_wait(&obj->cond, &obj->mtx);
        }
        printNumber(i);
        obj->turn = 0;
        pthread_cond_broadcast(&obj->cond);
        pthread_mutex_unlock(&obj->mtx);
    }
}

// Thread argument structure
typedef struct {
    ZeroEvenOdd* zeo;
    void (*printNumber)(int);
} ThreadArgs;

// Wrapper functions for threads
void* zero_thread_func(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    zero(t_args->zeo, t_args->printNumber);
    return NULL;
}

void* even_thread_func(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    even(t_args->zeo, t_args->printNumber);
    return NULL;
}

void* odd_thread_func(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    odd(t_args->zeo, t_args->printNumber);
    return NULL;
}

void print_number(int x) {
    printf("%d", x);
}

void runTest(int n) {
    printf("Test Case for n = %d\n", n);
    printf("Output: ");

    ZeroEvenOdd zeo;
    zeo.n = n;
    zeo.turn = 0;
    pthread_mutex_init(&zeo.mtx, NULL);
    pthread_cond_init(&zeo.cond, NULL);

    ThreadArgs args = {&zeo, print_number};

    pthread_t threadA, threadB, threadC;
    pthread_create(&threadA, NULL, zero_thread_func, &args);
    pthread_create(&threadB, NULL, even_thread_func, &args);
    pthread_create(&threadC, NULL, odd_thread_func, &args);

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadC, NULL);

    pthread_mutex_destroy(&zeo.mtx);
    pthread_cond_destroy(&zeo.cond);
    
    printf("\n\n");
}

int main() {
    runTest(2);
    runTest(5);
    runTest(1);
    runTest(6);
    runTest(10);
    return 0;
}