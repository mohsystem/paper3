#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int n;
    volatile int state; // 0: zero, 1: odd, 2: even
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} ZeroEvenOdd;

typedef void (*PrintNumber)(int);

typedef struct {
    ZeroEvenOdd* obj;
    PrintNumber printNumber;
} ThreadArgs;

ZeroEvenOdd* zeroEvenOddCreate(int n) {
    if (n < 1 || n > 1000) {
        fprintf(stderr, "Error: n must be between 1 and 1000.\n");
        return NULL;
    }
    ZeroEvenOdd* obj = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (obj == NULL) return NULL;
    obj->n = n;
    obj->state = 0;
    pthread_mutex_init(&obj->mtx, NULL);
    pthread_cond_init(&obj->cv, NULL);
    return obj;
}

void zeroEvenOddFree(ZeroEvenOdd* obj) {
    if (obj == NULL) return;
    pthread_mutex_destroy(&obj->mtx);
    pthread_cond_destroy(&obj->cv);
    free(obj);
}

void* zero_thread_func(void* args_ptr) {
    ThreadArgs* args = (ThreadArgs*)args_ptr;
    ZeroEvenOdd* obj = args->obj;
    PrintNumber printNumber = args->printNumber;

    for (int i = 1; i <= obj->n; ++i) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 0) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        printNumber(0);
        if (i % 2 != 0) {
            obj->state = 1;
        } else {
            obj->state = 2;
        }
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

void* even_thread_func(void* args_ptr) {
    ThreadArgs* args = (ThreadArgs*)args_ptr;
    ZeroEvenOdd* obj = args->obj;
    PrintNumber printNumber = args->printNumber;

    for (int i = 2; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 2) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        printNumber(i);
        obj->state = 0;
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

void* odd_thread_func(void* args_ptr) {
    ThreadArgs* args = (ThreadArgs*)args_ptr;
    ZeroEvenOdd* obj = args->obj;
    PrintNumber printNumber = args->printNumber;

    for (int i = 1; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 1) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        printNumber(i);
        obj->state = 0;
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

void print_number_func(int x) {
    printf("%d", x);
}

void runTest(int n) {
    printf("Test case n = %d\n", n);
    ZeroEvenOdd* zeo = zeroEvenOddCreate(n);
    if (zeo == NULL) {
        return;
    }

    ThreadArgs args = { .obj = zeo, .printNumber = print_number_func };

    pthread_t tA, tB, tC;
    pthread_create(&tA, NULL, zero_thread_func, &args);
    pthread_create(&tB, NULL, even_thread_func, &args);
    pthread_create(&tC, NULL, odd_thread_func, &args);

    pthread_join(tA, NULL);
    pthread_join(tB, NULL);
    pthread_join(tC, NULL);

    printf("\n\n");
    zeroEvenOddFree(zeo);
}

int main() {
    int testCases[] = {1, 2, 5, 6, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTestCases; ++i) {
        runTest(testCases[i]);
    }
    return 0;
}