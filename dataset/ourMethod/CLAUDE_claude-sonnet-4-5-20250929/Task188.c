
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    int n;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    bool fooTurn; // Synchronization flag for alternating execution
    char* result; // Shared result buffer
    size_t capacity; // Buffer capacity to prevent overflow
    size_t length; // Current string length
} FooBar;

typedef struct {
    FooBar* fb;
} ThreadArg;

// Safe string append with bounds checking
static bool append_safe(FooBar* fb, const char* str) {
    if (fb == NULL || str == NULL) {
        return false;
    }
    
    size_t str_len = strlen(str);
    // Check for potential overflow before append
    if (fb->length + str_len >= fb->capacity) {
        // Expand buffer with overflow protection
        size_t new_capacity = fb->capacity * 2;
        if (new_capacity < fb->capacity) { // Overflow check
            return false;
        }
        
        char* new_result = (char*)realloc(fb->result, new_capacity);
        if (new_result == NULL) {
            return false; // Memory allocation failed
        }
        fb->result = new_result;
        fb->capacity = new_capacity;
    }
    
    // Safe copy with bounds check
    memcpy(fb->result + fb->length, str, str_len);
    fb->length += str_len;
    fb->result[fb->length] = '\\0'; // Ensure null termination
    return true;
}

// Initialize FooBar with validation
FooBar* foobar_create(int n) {
    // Input validation per constraints
    if (n < 1 || n > 1000) {
        return NULL;
    }
    
    FooBar* fb = (FooBar*)calloc(1, sizeof(FooBar));
    if (fb == NULL) {
        return NULL;
    }
    
    fb->n = n;
    fb->fooTurn = true;
    
    // Calculate required capacity with overflow check
    size_t required = (size_t)n * 6 + 1; // "foobar" = 6 chars per iteration + null
    if (required < (size_t)n) { // Overflow check
        free(fb);
        return NULL;
    }
    
    fb->capacity = required;
    fb->result = (char*)calloc(fb->capacity, sizeof(char));
    if (fb->result == NULL) {
        free(fb);
        return NULL;
    }
    fb->length = 0;
    fb->result[0] = '\\0';
    
    // Initialize mutex and condition variable
    if (pthread_mutex_init(&fb->mtx, NULL) != 0) {
        free(fb->result);
        free(fb);
        return NULL;
    }
    
    if (pthread_cond_init(&fb->cv, NULL) != 0) {
        pthread_mutex_destroy(&fb->mtx);
        free(fb->result);
        free(fb);
        return NULL;
    }
    
    return fb;
}

void* foo(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    FooBar* fb = targ->fb;
    
    for (int i = 0; i < fb->n; i++) {
        pthread_mutex_lock(&fb->mtx); // Acquire lock
        
        // Wait for foo's turn, prevents race conditions\n        while (!fb->fooTurn) {\n            pthread_cond_wait(&fb->cv, &fb->mtx);\n        }\n        \n        // Critical section: append "foo" safely\n        if (!append_safe(fb, "foo")) {\n            pthread_mutex_unlock(&fb->mtx);\n            return NULL;\n        }\n        \n        fb->fooTurn = false; // Signal bar's turn
        pthread_cond_signal(&fb->cv); // Wake bar thread
        pthread_mutex_unlock(&fb->mtx); // Release lock
    }
    
    return NULL;
}

void* bar(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    FooBar* fb = targ->fb;
    
    for (int i = 0; i < fb->n; i++) {
        pthread_mutex_lock(&fb->mtx); // Acquire lock
        
        // Wait for bar's turn, prevents race conditions\n        while (fb->fooTurn) {\n            pthread_cond_wait(&fb->cv, &fb->mtx);\n        }\n        \n        // Critical section: append "bar" safely\n        if (!append_safe(fb, "bar")) {\n            pthread_mutex_unlock(&fb->mtx);\n            return NULL;\n        }\n        \n        fb->fooTurn = true; // Signal foo's turn
        pthread_cond_signal(&fb->cv); // Wake foo thread
        pthread_mutex_unlock(&fb->mtx); // Release lock
    }
    
    return NULL;
}

void foobar_destroy(FooBar* fb) {
    if (fb == NULL) {
        return;
    }
    
    pthread_mutex_destroy(&fb->mtx);
    pthread_cond_destroy(&fb->cv);
    
    if (fb->result != NULL) {
        // Clear sensitive data before free
        memset(fb->result, 0, fb->capacity);
        free(fb->result);
    }
    
    free(fb);
}

int main(void) {
    pthread_t t1, t2;
    
    // Test case 1: n = 1
    FooBar* fb1 = foobar_create(1);
    if (fb1 != NULL) {
        ThreadArg arg1 = {fb1};
        pthread_create(&t1, NULL, foo, &arg1);
        pthread_create(&t2, NULL, bar, &arg1);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        printf("Test 1 (n=1): %s\\n", fb1->result);
        foobar_destroy(fb1);
    }
    
    // Test case 2: n = 2
    FooBar* fb2 = foobar_create(2);
    if (fb2 != NULL) {
        ThreadArg arg2 = {fb2};
        pthread_create(&t1, NULL, foo, &arg2);
        pthread_create(&t2, NULL, bar, &arg2);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        printf("Test 2 (n=2): %s\\n", fb2->result);
        foobar_destroy(fb2);
    }
    
    // Test case 3: n = 5
    FooBar* fb3 = foobar_create(5);
    if (fb3 != NULL) {
        ThreadArg arg3 = {fb3};
        pthread_create(&t1, NULL, foo, &arg3);
        pthread_create(&t2, NULL, bar, &arg3);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        printf("Test 3 (n=5): %s\\n", fb3->result);
        foobar_destroy(fb3);
    }
    
    // Test case 4: n = 10
    FooBar* fb4 = foobar_create(10);
    if (fb4 != NULL) {
        ThreadArg arg4 = {fb4};
        pthread_create(&t1, NULL, foo, &arg4);
        pthread_create(&t2, NULL, bar, &arg4);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        printf("Test 4 (n=10): %s\\n", fb4->result);
        foobar_destroy(fb4);
    }
    
    // Test case 5: n = 1000
    FooBar* fb5 = foobar_create(1000);
    if (fb5 != NULL) {
        ThreadArg arg5 = {fb5};
        pthread_create(&t1, NULL, foo, &arg5);
        pthread_create(&t2, NULL, bar, &arg5);
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        printf("Test 5 (n=1000): Length = %zu\\n", fb5->length);
        foobar_destroy(fb5);
    }
    
    return 0;
}
