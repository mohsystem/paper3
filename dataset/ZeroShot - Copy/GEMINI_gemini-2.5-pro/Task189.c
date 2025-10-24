#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Note: This code uses POSIX threads (pthreads).
// To compile, link the pthread library, e.g., using `gcc -pthread your_file.c`.

// A function pointer type for printNumber
typedef void (*PrintNumberFunc)(int);

// Structure to hold shared data and synchronization primitives
typedef struct {
    int n;
    volatile int current;
    volatile int state; // 0 for zero, 1 for odd, 2 for even
    pthread_mutex_t mtx;
    pthread_cond_t cv;
} ZeroEvenOdd;

// Functions for the ZeroEvenOdd 'class'
ZeroEvenOdd* zeroEvenOddCreate(int n);
void zeroEvenOddFree(ZeroEvenOdd* obj);
void zero(ZeroEvenOdd* obj, PrintNumberFunc printNumber);
void even(ZeroEvenOdd* obj, PrintNumberFunc printNumber);
void odd(ZeroEvenOdd* obj, PrintNumberFunc printNumber);

// Function implementations
ZeroEvenOdd* zeroEvenOddCreate(int n) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (!obj) return NULL;
    obj->n = n;
    obj->current = 1;
    obj->state = 0;
    if (pthread_mutex_init(&obj->mtx, NULL) != 0) {
        free(obj);
        return NULL;
    }
    if (pthread_cond_init(&obj->cv, NULL) != 0) {
        pthread_mutex_destroy(&obj->mtx);
        free(obj);
        return NULL;
    }
    return obj;
}

void zeroEvenOddFree(ZeroEvenOdd* obj) {
    if (!obj) return;
    pthread_mutex_destroy(&obj->mtx);
    pthread_cond_destroy(&obj->cv);
    free(obj);
}

void zero(ZeroEvenOdd* obj, PrintNumberFunc printNumber) {
    for (int i = 0; i < obj->n; ++i) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 0) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        
        printNumber(0);
        
        if (obj->current % 2 != 0) {
            obj->state = 1;
        } else {
            obj->state = 2;
        }
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
}

void even(ZeroEvenOdd* obj, PrintNumberFunc printNumber) {
    for (int i = 2; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 2) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        
        printNumber(obj->current);
        obj->current++;
        obj->state = 0;
        
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
}

void odd(ZeroEvenOdd* obj, PrintNumberFunc printNumber) {
    for (int i = 1; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 1) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        
        printNumber(obj->current);
        obj->current++;
        obj->state = 0;
        
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
}

// Wrapper for thread arguments
typedef struct {
    ZeroEvenOdd* zeo;
    PrintNumberFunc printFunc;
} ThreadArgs;

// Thread runner functions
void* zero_thread_runner(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    zero(t_args->zeo, t_args->printFunc);
    return NULL;
}

void* even_thread_runner(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    even(t_args->zeo, t_args->printFunc);
    return NULL;
}

void* odd_thread_runner(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    odd(t_args->zeo, t_args->printFunc);
    return NULL;
}

// Concrete printNumber function
void print_number_c(int x) {
    printf("%d", x);
    fflush(stdout);
}

void run_test_case(int n) {
    printf("Test Case n = %d\n", n);
    printf("Output: ");
    
    ZeroEvenOdd* zeo = zeroEvenOddCreate(n);
    if (!zeo) {
        fprintf(stderr, "Failed to create ZeroEvenOdd object\n");
        return;
    }
    
    ThreadArgs args = {zeo, print_number_c};
    
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, zero_thread_runner, &args);
    pthread_create(&t2, NULL, even_thread_runner, &args);
    pthread_create(&t3, NULL, odd_thread_runner, &args);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf("\n\n");
    
    zeroEvenOddFree(zeo);
}

int main() {
    int testCases[] = {1, 2, 5, 6, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    
    for (int i = 0; i < numTestCases; ++i) {
        run_test_case(testCases[i]);
    }
    
    return 0;
}