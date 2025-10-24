
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    int n;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    int state; // 0: zero, 1: odd, 2: even
    int current; // Current number (1 to n)
} ZeroEvenOdd;

typedef void (*PrintNumber)(int);

// Initialize ZeroEvenOdd instance
ZeroEvenOdd* ZeroEvenOdd_create(int n) {
    // Validate input: 1 <= n <= 1000
    if (n < 1 || n > 1000) {
        fprintf(stderr, "Error: n must be between 1 and 1000\\n");
        return NULL;
    }
    
    ZeroEvenOdd* obj = (ZeroEvenOdd*)malloc(sizeof(ZeroEvenOdd));
    if (obj == NULL) {
        fprintf(stderr, "Error: memory allocation failed\\n");
        return NULL;
    }
    
    obj->n = n;
    obj->state = 0;
    obj->current = 1;
    
    // Initialize mutex and condition variable
    if (pthread_mutex_init(&obj->mtx, NULL) != 0) {
        free(obj);
        fprintf(stderr, "Error: mutex initialization failed\\n");
        return NULL;
    }
    
    if (pthread_cond_init(&obj->cv, NULL) != 0) {
        pthread_mutex_destroy(&obj->mtx);
        free(obj);
        fprintf(stderr, "Error: condition variable initialization failed\\n");
        return NULL;
    }
    
    return obj;
}

// Cleanup resources
void ZeroEvenOdd_destroy(ZeroEvenOdd* obj) {
    if (obj == NULL) {
        return;
    }
    
    pthread_mutex_destroy(&obj->mtx);
    pthread_cond_destroy(&obj->cv);
    free(obj);
}

// Thread A function
void zero(ZeroEvenOdd* obj, PrintNumber printNumber) {
    if (obj == NULL || printNumber == NULL) {
        return;
    }
    
    for (int i = 0; i < obj->n; ++i) {
        pthread_mutex_lock(&obj->mtx);
        
        // Wait for state == 0
        while (obj->state != 0) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        
        // Print zero
        printNumber(0);
        
        // Determine next state
        if (obj->current % 2 == 1) {
            obj->state = 1; // odd's turn\n        } else {\n            obj->state = 2; // even's turn
        }
        
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
}

// Thread B function
void even(ZeroEvenOdd* obj, PrintNumber printNumber) {
    if (obj == NULL || printNumber == NULL) {
        return;
    }
    
    for (int i = 2; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        
        // Wait for state == 2 and current == i
        while (obj->state != 2 || obj->current != i) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        
        // Print even number
        printNumber(i);
        
        // Move to next number
        obj->current++;
        
        // Check bounds
        if (obj->current <= obj->n) {
            obj->state = 0;
        }
        
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
}

// Thread C function
void odd(ZeroEvenOdd* obj, PrintNumber printNumber) {
    if (obj == NULL || printNumber == NULL) {
        return;
    }
    
    for (int i = 1; i <= obj->n; i += 2) {
        pthread_mutex_lock(&obj->mtx);
        
        // Wait for state == 1 and current == i
        while (obj->state != 1 || obj->current != i) {
            pthread_cond_wait(&obj->cv, &obj->mtx);
        }
        
        // Print odd number
        printNumber(i);
        
        // Move to next number
        obj->current++;
        
        // Check bounds
        if (obj->current <= obj->n) {
            obj->state = 0;
        }
        
        pthread_cond_broadcast(&obj->cv);
        pthread_mutex_unlock(&obj->mtx);
    }
}

// Thread argument structure
typedef struct {
    ZeroEvenOdd* obj;
    PrintNumber printNumber;
} ThreadArg;

// Thread wrapper functions
void* zero_thread(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    zero(targ->obj, targ->printNumber);
    return NULL;
}

void* even_thread(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    even(targ->obj, targ->printNumber);
    return NULL;
}

void* odd_thread(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    odd(targ->obj, targ->printNumber);
    return NULL;
}

// Test print function
void test_print(int x) {
    printf("%d", x);
}

// Test helper function
void run_test(int n, const char* expected) {
    printf("Test (n=%d): ", n);
    fflush(stdout);
    
    ZeroEvenOdd* zeo = ZeroEvenOdd_create(n);
    if (zeo == NULL) {
        printf("FAILED (creation error)\\n");
        return;
    }
    
    pthread_t t1, t2, t3;
    ThreadArg arg = {zeo, test_print};
    
    if (pthread_create(&t1, NULL, zero_thread, &arg) != 0 ||
        pthread_create(&t2, NULL, even_thread, &arg) != 0 ||
        pthread_create(&t3, NULL, odd_thread, &arg) != 0) {
        printf("FAILED (thread creation error)\\n");
        ZeroEvenOdd_destroy(zeo);
        return;
    }
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    printf(" (expected: %s)\\n", expected);
    
    ZeroEvenOdd_destroy(zeo);
}

int main(void) {
    run_test(2, "0102");
    run_test(5, "0102030405");
    run_test(1, "01");
    run_test(10, "010203040506070809010");
    run_test(3, "010203");
    
    return 0;
}
