#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

// Note: This code uses POSIX threads and semaphores.
// Compile with: gcc -o your_program_name your_source_file.c -lpthread

// The function that prints to the console
void consolePrintNumber(int x) {
    printf("%d", x);
}

// Struct to hold shared data, mimicking a class instance
typedef struct {
    int n;
    sem_t sem_zero;
    sem_t sem_even;
    sem_t sem_odd;
} ZeroEvenOdd;

// Thread function for printing zeros
void* zero(void* arg) {
    ZeroEvenOdd* zeo = (ZeroEvenOdd*)arg;
    for (int i = 1; i <= zeo->n; ++i) {
        sem_wait(&zeo->sem_zero);
        consolePrintNumber(0);
        if (i % 2 != 0) { // Next number is odd
            sem_post(&zeo->sem_odd);
        } else { // Next number is even
            sem_post(&zeo->sem_even);
        }
    }
    return NULL;
}

// Thread function for printing even numbers
void* even(void* arg) {
    ZeroEvenOdd* zeo = (ZeroEvenOdd*)arg;
    for (int i = 2; i <= zeo->n; i += 2) {
        sem_wait(&zeo->sem_even);
        consolePrintNumber(i);
        sem_post(&zeo->sem_zero);
    }
    return NULL;
}

// Thread function for printing odd numbers
void* odd(void* arg) {
    ZeroEvenOdd* zeo = (ZeroEvenOdd*)arg;
    for (int i = 1; i <= zeo->n; i += 2) {
        sem_wait(&zeo->sem_odd);
        consolePrintNumber(i);
        sem_post(&zeo->sem_zero);
    }
    return NULL;
}

// Helper to create and initialize the ZeroEvenOdd struct
ZeroEvenOdd* zeroEvenOddCreate(int n) {
    ZeroEvenOdd* zeo = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (zeo == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }
    zeo->n = n;
    sem_init(&zeo->sem_zero, 0, 1);
    sem_init(&zeo->sem_even, 0, 0);
    sem_init(&zeo->sem_odd, 0, 0);
    return zeo;
}

// Helper to clean up resources
void zeroEvenOddFree(ZeroEvenOdd* zeo) {
    if (zeo != NULL) {
        sem_destroy(&zeo->sem_zero);
        sem_destroy(&zeo->sem_even);
        sem_destroy(&zeo->sem_odd);
        free(zeo);
    }
}

void run_test(int n) {
    printf("Test Case n = %d:\n", n);
    ZeroEvenOdd* zeo = zeroEvenOddCreate(n);
    if (zeo == NULL) return;

    pthread_t thread_a, thread_b, thread_c;
    
    pthread_create(&thread_a, NULL, zero, zeo);
    pthread_create(&thread_b, NULL, even, zeo);
    pthread_create(&thread_c, NULL, odd, zeo);

    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);

    printf("\n\n");
    zeroEvenOddFree(zeo);
}

// Main function to run all test cases, named to align with other examples
void Task196_main() {
    int testCases[] = {2, 5, 1, 6, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTestCases; ++i) {
        run_test(testCases[i]);
    }
}

int main() {
    Task196_main();
    return 0;
}