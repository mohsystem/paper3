
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// Structure to hold ZeroEvenOdd state
typedef struct {
    int n;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    int state; // 0: zero turn, 1: odd turn, 2: even turn
    int current; // current number to print
} ZeroEvenOdd;

// Thread argument structure
typedef struct {
    ZeroEvenOdd* zeo;
    void (*printNumber)(int);
} ThreadArg;

// Initialize ZeroEvenOdd with validated input
ZeroEvenOdd* createZeroEvenOdd(int n) {
    // Validate input range as per constraints
    if (n < 1 || n > 1000) {
        fprintf(stderr, "Error: n must be between 1 and 1000\\n");
        return NULL;
    }
    
    ZeroEvenOdd* zeo = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (zeo == NULL) {
        fprintf(stderr, "Error: memory allocation failed\\n");
        return NULL;
    }
    
    zeo->n = n;
    zeo->state = 0;
    zeo->current = 1;
    
    // Initialize mutex and condition variable
    if (pthread_mutex_init(&zeo->mtx, NULL) != 0) {
        free(zeo);
        fprintf(stderr, "Error: mutex initialization failed\\n");
        return NULL;
    }
    
    if (pthread_cond_init(&zeo->cv, NULL) != 0) {
        pthread_mutex_destroy(&zeo->mtx);
        free(zeo);
        fprintf(stderr, "Error: condition variable initialization failed\\n");
        return NULL;
    }
    
    return zeo;
}

// Cleanup resources
void destroyZeroEvenOdd(ZeroEvenOdd* zeo) {
    if (zeo != NULL) {
        pthread_mutex_destroy(&zeo->mtx);
        pthread_cond_destroy(&zeo->cv);
        free(zeo);
    }
}

// Zero thread function
void* zero(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    if (targ == NULL || targ->zeo == NULL) return NULL;
    
    ZeroEvenOdd* zeo = targ->zeo;
    void (*printNumber)(int) = targ->printNumber;
    
    for (int i = 1; i <= zeo->n; ++i) {
        pthread_mutex_lock(&zeo->mtx);
        
        // Wait until it's zero's turn
        while (zeo->state != 0) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        
        // Print zero
        printNumber(0);
        
        // Set next state based on current number
        zeo->state = (zeo->current % 2 == 1) ? 1 : 2;
        
        pthread_cond_broadcast(&zeo->cv);
        pthread_mutex_unlock(&zeo->mtx);
    }
    
    return NULL;
}

// Even thread function
void* even(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    if (targ == NULL || targ->zeo == NULL) return NULL;
    
    ZeroEvenOdd* zeo = targ->zeo;
    void (*printNumber)(int) = targ->printNumber;
    
    for (int i = 2; i <= zeo->n; i += 2) {
        pthread_mutex_lock(&zeo->mtx);
        
        // Wait until it's even's turn and current matches
        while (zeo->state != 2 || zeo->current != i) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        
        // Print the even number
        printNumber(i);
        
        // Move to next number
        zeo->current++;
        zeo->state = 0;
        
        pthread_cond_broadcast(&zeo->cv);
        pthread_mutex_unlock(&zeo->mtx);
    }
    
    return NULL;
}

// Odd thread function
void* odd(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    if (targ == NULL || targ->zeo == NULL) return NULL;
    
    ZeroEvenOdd* zeo = targ->zeo;
    void (*printNumber)(int) = targ->printNumber;
    
    for (int i = 1; i <= zeo->n; i += 2) {
        pthread_mutex_lock(&zeo->mtx);
        
        // Wait until it's odd's turn and current matches
        while (zeo->state != 1 || zeo->current != i) {
            pthread_cond_wait(&zeo->cv, &zeo->mtx);
        }
        
        // Print the odd number
        printNumber(i);
        
        // Move to next number
        zeo->current++;
        zeo->state = 0;
        
        pthread_cond_broadcast(&zeo->cv);
        pthread_mutex_unlock(&zeo->mtx);
    }
    
    return NULL;
}

// Test print function
void testPrintNumber(int x) {
    printf("%d", x);
}

// Run a test case
void runTest(int n, const char* expected) {
    printf("Test n=%d: ", n);
    fflush(stdout);
    
    ZeroEvenOdd* zeo = createZeroEvenOdd(n);
    if (zeo == NULL) {
        printf("FAIL (creation failed)\\n");
        return;
    }
    
    pthread_t t1, t2, t3;
    ThreadArg arg1 = {zeo, testPrintNumber};
    ThreadArg arg2 = {zeo, testPrintNumber};
    ThreadArg arg3 = {zeo, testPrintNumber};
    
    pthread_create(&t1, NULL, zero, &arg1);
    pthread_create(&t2, NULL, even, &arg2);
    pthread_create(&t3, NULL, odd, &arg3);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf(" - Expected: %s\\n", expected);
    
    destroyZeroEvenOdd(zeo);
}

int main() {
    // Test case 1: n = 2
    runTest(2, "0102");
    
    // Test case 2: n = 5
    runTest(5, "0102030405");
    
    // Test case 3: n = 1
    runTest(1, "01");
    
    // Test case 4: n = 3
    runTest(3, "010203");
    
    // Test case 5: n = 10
    runTest(10, "010203040506070809010");
    
    return 0;
}
