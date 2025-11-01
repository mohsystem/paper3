
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define BUFFER_SIZE 32
#define NUM_STRINGS 3

/* Structure to hold the three strings */
typedef struct {
    char str_a[BUFFER_SIZE];
    char str_b[BUFFER_SIZE];
    char str_c[BUFFER_SIZE];
} ThreeStrings;

/* Initialize random number generator with time-based seed
 * Note: For production use, consider using /dev/urandom on Unix systems
 * or CryptGenRandom on Windows for cryptographically secure randomness
 */
static int random_initialized = 0;

void init_random(void) {
    if (!random_initialized) {
        /* Seed with current time - not cryptographically secure but acceptable for demo */
        srand((unsigned int)time(NULL));
        random_initialized = 1;
    }
}

/**
 * Generates a random float between 0.0 and 1.0
 * Returns float value
 */
float generate_random_float(void) {
    init_random();
    /* Generate random integer and normalize to [0.0, 1.0) */
    return (float)rand() / (float)RAND_MAX;
}

/**
 * Converts float to string with bounds checking
 * Returns 0 on success, -1 on error
 */
int float_to_string(float value, char *buffer, size_t buffer_size) {
    /* Validate buffer pointer and size */
    if (buffer == NULL || buffer_size < BUFFER_SIZE) {
        return -1;
    }
    
    /* Initialize buffer to prevent use of uninitialized memory */
    memset(buffer, 0, buffer_size);
    
    /* Use snprintf for bounds-safe string formatting */
    int result = snprintf(buffer, buffer_size, "%.6f", value);
    
    /* Check for formatting error or truncation */
    if (result < 0 || (size_t)result >= buffer_size) {
        return -1;
    }
    
    /* Ensure null termination */
    buffer[buffer_size - 1] = '\\0';
    
    return 0;
}

/**
 * Generates three random floats and converts them to strings
 * Returns pointer to ThreeStrings structure on success, NULL on error
 * Caller is responsible for freeing the returned structure
 */
ThreeStrings* generate_three_random_float_strings(void) {
    /* Allocate memory for result structure */
    ThreeStrings *result = (ThreeStrings*)calloc(1, sizeof(ThreeStrings));
    if (result == NULL) {
        return NULL;
    }
    
    /* Generate three random floats */
    float float_a = generate_random_float();
    float float_b = generate_random_float();
    float float_c = generate_random_float();
    
    /* Convert to strings with error checking */
    if (float_to_string(float_a, result->str_a, BUFFER_SIZE) != 0 ||
        float_to_string(float_b, result->str_b, BUFFER_SIZE) != 0 ||
        float_to_string(float_c, result->str_c, BUFFER_SIZE) != 0) {
        /* Cleanup on error */
        free(result);
        return NULL;
    }
    
    return result;
}

int main(void) {
    ThreeStrings *result = NULL;
    
    /* Test case 1 */
    result = generate_three_random_float_strings();
    if (result != NULL) {
        printf("Test 1 - str_a: %s, str_b: %s, str_c: %s\\n", 
               result->str_a, result->str_b, result->str_c);
        free(result);
        result = NULL;
    }
    
    /* Test case 2 */
    result = generate_three_random_float_strings();
    if (result != NULL) {
        printf("Test 2 - str_a: %s, str_b: %s, str_c: %s\\n", 
               result->str_a, result->str_b, result->str_c);
        free(result);
        result = NULL;
    }
    
    /* Test case 3 */
    result = generate_three_random_float_strings();
    if (result != NULL) {
        printf("Test 3 - str_a: %s, str_b: %s, str_c: %s\\n", 
               result->str_a, result->str_b, result->str_c);
        free(result);
        result = NULL;
    }
    
    /* Test case 4 */
    result = generate_three_random_float_strings();
    if (result != NULL) {
        printf("Test 4 - str_a: %s, str_b: %s, str_c: %s\\n", 
               result->str_a, result->str_b, result->str_c);
        free(result);
        result = NULL;
    }
    
    /* Test case 5 */
    result = generate_three_random_float_strings();
    if (result != NULL) {
        printf("Test 5 - str_a: %s, str_b: %s, str_c: %s\\n", 
               result->str_a, result->str_b, result->str_c);
        free(result);
        result = NULL;
    }
    
    return 0;
}
