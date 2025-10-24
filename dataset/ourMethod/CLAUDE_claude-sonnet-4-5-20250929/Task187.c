
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

// Thread synchronization structure with proper initialization
typedef struct {
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    int hydrogen_count;
    int oxygen_count;
    char* result;
    size_t result_size;
    size_t result_capacity;
} H2O;

// Thread argument structure with validated data
typedef struct {
    H2O* h2o;
    char atom_type;
} ThreadArg;

// Initialize H2O structure with proper error checking
bool h2o_init(H2O* h2o) {
    // Validate input pointer - prevent null pointer dereference
    if (h2o == NULL) {
        return false;
    }
    
    // Initialize mutex with error checking
    if (pthread_mutex_init(&h2o->mtx, NULL) != 0) {
        return false;
    }
    
    // Initialize condition variable with error checking
    if (pthread_cond_init(&h2o->cv, NULL) != 0) {
        pthread_mutex_destroy(&h2o->mtx);
        return false;
    }
    
    // Initialize counters - atomic operations protected by mutex
    h2o->hydrogen_count = 0;
    h2o->oxygen_count = 0;
    
    // Allocate result buffer with bounds checking
    h2o->result_capacity = 64;
    h2o->result = (char*)calloc(h2o->result_capacity, sizeof(char));
    if (h2o->result == NULL) {
        pthread_cond_destroy(&h2o->cv);
        pthread_mutex_destroy(&h2o->mtx);
        return false;
    }
    
    h2o->result_size = 0;
    h2o->result[0] = '\\0'; // Ensure null termination
    
    return true;
}

// Cleanup H2O structure - prevents memory leaks
void h2o_destroy(H2O* h2o) {
    if (h2o == NULL) {
        return;
    }
    
    // Free allocated memory - check for null before free
    if (h2o->result != NULL) {
        // Clear sensitive data before free (if applicable)
        memset(h2o->result, 0, h2o->result_capacity);
        free(h2o->result);
        h2o->result = NULL;
    }
    
    // Destroy synchronization primitives
    pthread_cond_destroy(&h2o->cv);
    pthread_mutex_destroy(&h2o->mtx);
}

// Append character to result with bounds checking
bool append_result(H2O* h2o, char c) {
    // Validate input - prevent null pointer dereference
    if (h2o == NULL || h2o->result == NULL) {
        return false;
    }
    
    // Lock mutex for thread-safe access
    if (pthread_mutex_lock(&h2o->mtx) != 0) {
        return false;
    }
    
    // Bounds check: ensure capacity before append
    if (h2o->result_size + 1 >= h2o->result_capacity) {
        // Reallocate with overflow check
        size_t new_capacity = h2o->result_capacity * 2;
        if (new_capacity < h2o->result_capacity) {
            pthread_mutex_unlock(&h2o->mtx);
            return false; // Overflow detected
        }
        
        char* new_result = (char*)realloc(h2o->result, new_capacity);
        if (new_result == NULL) {
            pthread_mutex_unlock(&h2o->mtx);
            return false;
        }
        
        h2o->result = new_result;
        h2o->result_capacity = new_capacity;
    }
    
    // Append character with null termination
    h2o->result[h2o->result_size] = c;
    h2o->result_size++;
    h2o->result[h2o->result_size] = '\\0';
    
    pthread_mutex_unlock(&h2o->mtx);
    return true;
}

// Hydrogen thread function with synchronization
void* hydrogen_thread(void* arg) {
    // Validate input pointer
    if (arg == NULL) {
        return NULL;
    }
    
    ThreadArg* thread_arg = (ThreadArg*)arg;
    H2O* h2o = thread_arg->h2o;
    
    if (h2o == NULL) {
        return NULL;
    }
    
    // Lock mutex for atomic operations
    pthread_mutex_lock(&h2o->mtx);
    
    // Increment hydrogen count - protected by mutex
    h2o->hydrogen_count++;
    
    // Wait until we have enough atoms to form complete molecule
    while (!(h2o->hydrogen_count >= 2 && h2o->oxygen_count >= 1)) {
        pthread_cond_wait(&h2o->cv, &h2o->mtx);
    }
    
    // Unlock before append to avoid deadlock
    pthread_mutex_unlock(&h2o->mtx);
    
    // Release hydrogen
    append_result(h2o, 'H');
    
    // Lock again for count update
    pthread_mutex_lock(&h2o->mtx);
    
    // Decrement hydrogen count
    h2o->hydrogen_count--;
    
    // If molecule complete, decrement oxygen and notify
    if (h2o->hydrogen_count == 0 && h2o->oxygen_count == 1) {
        h2o->oxygen_count--;
        pthread_cond_broadcast(&h2o->cv);
    }
    
    pthread_mutex_unlock(&h2o->mtx);
    
    return NULL;
}

// Oxygen thread function with synchronization
void* oxygen_thread(void* arg) {
    // Validate input pointer
    if (arg == NULL) {
        return NULL;
    }
    
    ThreadArg* thread_arg = (ThreadArg*)arg;
    H2O* h2o = thread_arg->h2o;
    
    if (h2o == NULL) {
        return NULL;
    }
    
    // Lock mutex for atomic operations
    pthread_mutex_lock(&h2o->mtx);
    
    // Increment oxygen count - protected by mutex
    h2o->oxygen_count++;
    
    // Wait until we have enough hydrogen atoms
    while (!(h2o->hydrogen_count >= 2)) {
        pthread_cond_wait(&h2o->cv, &h2o->mtx);
    }
    
    // Unlock before append to avoid deadlock
    pthread_mutex_unlock(&h2o->mtx);
    
    // Release oxygen
    append_result(h2o, 'O');
    
    // Lock again for notification
    pthread_mutex_lock(&h2o->mtx);
    
    // Notify waiting threads
    pthread_cond_broadcast(&h2o->cv);
    
    pthread_mutex_unlock(&h2o->mtx);
    
    return NULL;
}

// Build water molecule with input validation
char* build_molecule(const char* water) {
    // Validate input pointer - prevent null pointer dereference
    if (water == NULL) {
        return NULL;
    }
    
    size_t len = strlen(water);
    
    // Validate length constraint
    if (len == 0 || len % 3 != 0) {
        return NULL;
    }
    
    // Validate length bounds
    if (len > 1000) { // Reasonable upper limit
        return NULL;
    }
    
    // Count atoms with bounds-checked iteration
    int h_count = 0, o_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (water[i] == 'H') {
            h_count++;
        } else if (water[i] == 'O') {
            o_count++;
        } else {
            return NULL; // Invalid character
        }
    }
    
    // Validate H/O ratio
    if (h_count != 2 * o_count) {
        return NULL;
    }
    
    // Initialize H2O structure
    H2O h2o;
    if (!h2o_init(&h2o)) {
        return NULL;
    }
    
    // Allocate thread arrays with overflow check
    if (len > SIZE_MAX / sizeof(pthread_t)) {
        h2o_destroy(&h2o);
        return NULL;
    }
    
    pthread_t* threads = (pthread_t*)calloc(len, sizeof(pthread_t));
    if (threads == NULL) {
        h2o_destroy(&h2o);
        return NULL;
    }
    
    ThreadArg* args = (ThreadArg*)calloc(len, sizeof(ThreadArg));
    if (args == NULL) {
        free(threads);
        h2o_destroy(&h2o);
        return NULL;
    }
    
    // Create threads with bounds checking
    for (size_t i = 0; i < len; i++) {
        args[i].h2o = &h2o;
        args[i].atom_type = water[i];
        
        void* (*thread_func)(void*) = (water[i] == 'H') ? hydrogen_thread : oxygen_thread;
        
        if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
            // Error handling: join already created threads
            for (size_t j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(args);
            free(threads);
            h2o_destroy(&h2o);
            return NULL;
        }
    }
    
    // Wait for all threads to complete
    for (size_t i = 0; i < len; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Duplicate result string with bounds check
    char* result = NULL;
    if (h2o.result != NULL && h2o.result_size > 0) {
        result = (char*)malloc(h2o.result_size + 1);
        if (result != NULL) {
            memcpy(result, h2o.result, h2o.result_size);
            result[h2o.result_size] = '\\0';
        }
    }
    
    // Cleanup resources
    free(args);
    free(threads);
    h2o_destroy(&h2o);
    
    return result;
}

int main() {
    // Test case 1
    char* result1 = build_molecule("HOH");
    if (result1 != NULL) {
        printf("Test 1: %s\\n", result1);
        free(result1);
    }
    
    // Test case 2
    char* result2 = build_molecule("OOHHHH");
    if (result2 != NULL) {
        printf("Test 2: %s\\n", result2);
        free(result2);
    }
    
    // Test case 3
    char* result3 = build_molecule("HHO");
    if (result3 != NULL) {
        printf("Test 3: %s\\n", result3);
        free(result3);
    }
    
    // Test case 4
    char* result4 = build_molecule("HHHHHHOOO");
    if (result4 != NULL) {
        printf("Test 4: %s\\n", result4);
        free(result4);
    }
    
    // Test case 5
    char* result5 = build_molecule("OHH");
    if (result5 != NULL) {
        printf("Test 5: %s\\n", result5);
        free(result5);
    }
    
    return 0;
}
