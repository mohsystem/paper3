#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

typedef void (*printNumberFunc)(int);

typedef struct {
    int n;
    sem_t zero_sem;
    sem_t even_sem;
    sem_t odd_sem;
} ZeroEvenOdd;

typedef struct {
    ZeroEvenOdd* obj;
    printNumberFunc printNumber;
} ThreadArgs;


ZeroEvenOdd* zeroEvenOddCreate(int n) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (obj) {
        obj->n = n;
        sem_init(&obj->zero_sem, 0, 1);
        sem_init(&obj->even_sem, 0, 0);
        sem_init(&obj->odd_sem, 0, 0);
    }
    return obj;
}

void zeroEvenOddFree(ZeroEvenOdd* obj) {
    if (obj) {
        sem_destroy(&obj->zero_sem);
        sem_destroy(&obj->even_sem);
        sem_destroy(&obj->odd_sem);
        free(obj);
    }
}

void zero(ZeroEvenOdd* obj, printNumberFunc printNumber) {
    for (int i = 1; i <= obj->n; i++) {
        sem_wait(&obj->zero_sem);
        printNumber(0);
        if (i % 2 != 0) {
            sem_post(&obj->odd_sem);
        } else {
            sem_post(&obj->even_sem);
        }
    }
}

void even(ZeroEvenOdd* obj, printNumberFunc printNumber) {
    for (int i = 2; i <= obj->n; i += 2) {
        sem_wait(&obj->even_sem);
        printNumber(i);
        sem_post(&obj->zero_sem);
    }
}

void odd(ZeroEvenOdd* obj, printNumberFunc printNumber) {
    for (int i = 1; i <= obj->n; i += 2) {
        sem_wait(&obj->odd_sem);
        printNumber(i);
        sem_post(&obj->zero_sem);
    }
}


void* zero_thread_func(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    zero(t_args->obj, t_args->printNumber);
    return NULL;
}
void* even_thread_func(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    even(t_args->obj, t_args->printNumber);
    return NULL;
}
void* odd_thread_func(void* args) {
    ThreadArgs* t_args = (ThreadArgs*)args;
    odd(t_args->obj, t_args->printNumber);
    return NULL;
}

void print_number_c(int x) {
    printf("%d", x);
}

void run_test_c(int n) {
    printf("Test Case n = %d: ", n);
    ZeroEvenOdd* zeo = zeroEvenOddCreate(n);

    ThreadArgs args = {zeo, print_number_c};
    
    pthread_t thread_a, thread_b, thread_c;
    
    pthread_create(&thread_a, NULL, zero_thread_func, &args);
    pthread_create(&thread_b, NULL, even_thread_func, &args);
    pthread_create(&thread_c, NULL, odd_thread_func, &args);
    
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);
    
    printf("\n\n");
    
    zeroEvenOddFree(zeo);
}

int main() {
    int test_cases[] = {1, 2, 5, 6, 10};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < num_tests; ++i) {
        run_test_c(test_cases[i]);
    }
    return 0;
}