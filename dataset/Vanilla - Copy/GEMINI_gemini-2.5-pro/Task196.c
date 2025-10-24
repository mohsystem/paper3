#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Note: This C code uses the POSIX threads (pthreads) library.
// To compile, you may need to link against it, e.g., using `gcc -pthread your_file.c -o your_program`

// Structure to hold shared state and synchronization primitives
typedef struct {
    int n;
    volatile int state; // 0 for zero, 1 for odd, 2 for even
    pthread_mutex_t mtx;
    pthread_cond_t cond;
} ZeroEvenOdd;

// Function to print a number, mimicking the problem's printNumber
void printNumber(int x) {
    printf("%d", x);
}

// Thread function for printing zeros
void* zero_thread(void* arg) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)arg;
    for (int i = 1; i <= obj->n; i++) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 0) {
            pthread_cond_wait(&obj->cond, &obj->mtx);
        }
        printNumber(0);
        if (i % 2 != 0) {
            obj->state = 1; // Next is odd
        } else {
            obj->state = 2; // Next is even
        }
        pthread_cond_broadcast(&obj->cond);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

// Thread function for printing even numbers
void* even_thread(void* arg) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)arg;
    for (int i = 2; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 2) {
            pthread_cond_wait(&obj->cond, &obj->mtx);
        }
        printNumber(i);
        obj->state = 0;
        pthread_cond_broadcast(&obj->cond);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

// Thread function for printing odd numbers
void* odd_thread(void* arg) {
    ZeroEvenOdd* obj = (ZeroEvenOdd*)arg;
    for (int i = 1; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        while (obj->state != 1) {
            pthread_cond_wait(&obj->cond, &obj->mtx);
        }
        printNumber(i);
        obj->state = 0;
        pthread_cond_broadcast(&obj->cond);
        pthread_mutex_unlock(&obj->mtx);
    }
    return NULL;
}

// Helper function to run a test case
void runTest(int n) {
    printf("Test case n = %d\n", n);
    printf("Output: ");
    
    ZeroEvenOdd obj;
    obj.n = n;
    obj.state = 0;
    
    pthread_mutex_init(&obj.mtx, NULL);
    pthread_cond_init(&obj.cond, NULL);
    
    pthread_t t_zero, t_even, t_odd;
    
    pthread_create(&t_zero, NULL, zero_thread, &obj);
    pthread_create(&t_even, NULL, even_thread, &obj);
    pthread_create(&t_odd, NULL, odd_thread, &obj);
    
    pthread_join(t_zero, NULL);
    pthread_join(t_even, NULL);
    pthread_join(t_odd, NULL);
    
    pthread_mutex_destroy(&obj.mtx);
    pthread_cond_destroy(&obj.cond);
    
    printf("\n\n");
}

// C doesn't have classes. The main function serves as the entry point for "Task196.c"
int main() {
    runTest(2);
    runTest(5);
    runTest(1);
    runTest(6);
    runTest(10);
    return 0;
}