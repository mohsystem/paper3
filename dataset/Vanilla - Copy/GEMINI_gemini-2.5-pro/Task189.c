#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// C does not have classes. The "Task189" naming convention is not applicable.
// The main function serves as the program's entry point.

void printNumber(int x) {
    printf("%d", x);
}

typedef struct {
    int n;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    int state; // 0 for zero, 1 for odd, 2 for even
    int current_num;
} ZeroEvenOdd;

ZeroEvenOdd* zeroEvenOddCreate(int n) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    obj->n = n;
    pthread_mutex_init(&obj->mtx, NULL);
    pthread_cond_init(&obj->cond, NULL);
    obj->state = 0;
    obj->current_num = 1;
    return obj;
}

void zeroEvenOddFree(ZeroEvenOdd* obj) {
    pthread_mutex_destroy(&obj->mtx);
    pthread_cond_destroy(&obj->cond);
    free(obj);
}

typedef struct {
    ZeroEvenOdd* zeo;
    void (*printNumber)(int);
} ThreadArgs;

void* zero(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    ZeroEvenOdd* zeo = args->zeo;
    
    for (int i = 0; i < zeo->n; ++i) {
        pthread_mutex_lock(&zeo->mtx);
        while (zeo->state != 0) {
            pthread_cond_wait(&zeo->cond, &zeo->mtx);
        }
        
        args->printNumber(0);
        
        if (zeo->current_num % 2 != 0) {
            zeo->state = 1;
        } else {
            zeo->state = 2;
        }
        pthread_cond_broadcast(&zeo->cond);
        pthread_mutex_unlock(&zeo->mtx);
    }
    return NULL;
}

void* even(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    ZeroEvenOdd* zeo = args->zeo;

    for (int i = 2; i <= zeo->n; i += 2) {
        pthread_mutex_lock(&zeo->mtx);
        while (zeo->state != 2) {
            pthread_cond_wait(&zeo->cond, &zeo->mtx);
        }
        
        args->printNumber(zeo->current_num);
        
        zeo->current_num++;
        zeo->state = 0;
        pthread_cond_broadcast(&zeo->cond);
        pthread_mutex_unlock(&zeo->mtx);
    }
    return NULL;
}

void* odd(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    ZeroEvenOdd* zeo = args->zeo;

    for (int i = 1; i <= zeo->n; i += 2) {
        pthread_mutex_lock(&zeo->mtx);
        while (zeo->state != 1) {
            pthread_cond_wait(&zeo->cond, &zeo->mtx);
        }
        
        args->printNumber(zeo->current_num);
        
        zeo->current_num++;
        zeo->state = 0;
        pthread_cond_broadcast(&zeo->cond);
        pthread_mutex_unlock(&zeo->mtx);
    }
    return NULL;
}

void runTest(int n) {
    printf("Running test for n = %d\n", n);
    ZeroEvenOdd* zeo = zeroEvenOddCreate(n);

    ThreadArgs args = { zeo, printNumber };

    pthread_t threadA, threadB, threadC;
    
    pthread_create(&threadA, NULL, zero, &args);
    pthread_create(&threadB, NULL, even, &args);
    pthread_create(&threadC, NULL, odd, &args);

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadC, NULL);

    zeroEvenOddFree(zeo);
    printf("\n\n");
}

int main() {
    int testCases[] = {2, 5, 1, 6, 7};
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; ++i) {
        runTest(testCases[i]);
    }

    return 0;
}