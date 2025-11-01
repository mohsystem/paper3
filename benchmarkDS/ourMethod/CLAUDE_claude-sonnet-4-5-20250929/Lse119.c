
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <sys/random.h>
#endif

/* Buffer size for float string representation */
#define BUFFER_SIZE 64

/**
 * Structure to hold the three random float strings
 */
typedef struct {
    char string1[BUFFER_SIZE];
    char string2[BUFFER_SIZE];
    char string3[BUFFER_SIZE];
} RandomFloatStrings;

/**
 * Generates a cryptographically secure random float between 0.0 and 1.0
 * 
 * Security measures:
 * - Uses platform-specific CSPRNG (getrandom on Linux, BCryptGenRandom on Windows)
 * - Falls back to time-based seeding only if CSPRNG unavailable
 * - Validates all random number generation results
 */
static float secure_random_float(void) {
    unsigned int random_value = 0;
    
#ifdef _WIN32
    /* Use BCryptGenRandom on Windows for cryptographically secure random */
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)&random_value, 
                                     sizeof(random_value), 
                                     BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status != 0) {
        /* Fallback: seed with time if CSPRNG fails */
        srand((unsigned int)time(NULL));
        random_value = (unsigned int)rand();
    }
#else
    /* Use getrandom on Linux/Unix for cryptographically secure random */
    ssize_t result = getrandom(&random_value, sizeof(random_value), 0);
    if (result != sizeof(random_value)) {
        /* Fallback: seed with time if getrandom fails */
        srand((unsigned int)time(NULL));
        random_value = (unsigned int)rand();
    }
#endif
    
    /* Convert to float in range [0.0, 1.0] */
    return (float)random_value / (float)UINT_MAX;
}

/**
 * Generates three random floats and converts them to strings.
 * Returns 0 on success, -1 on failure.
 * 
 * Security measures:
 * - Uses secure random number generation
 * - Uses snprintf instead of sprintf to prevent buffer overflow
 * - All buffers initialized to zero
 * - Validates snprintf return values to detect truncation
 * - Ensures null termination of all strings
 * - Checks all function return values
 */
int generateRandomFloatStrings(RandomFloatStrings* result) {
    /* Validate input pointer */
    if (result == NULL) {
        return -1;
    }
    
    /* Initialize all buffers to zero for safety */
    memset(result, 0, sizeof(RandomFloatStrings));
    
    /* Generate three cryptographically strong random floats */
    float random1 = secure_random_float();
    float random2 = secure_random_float();
    float random3 = secure_random_float();
    
    /* Convert to strings using snprintf (safe sprintf alternative) */
    /* snprintf guarantees null termination and prevents buffer overflow */
    int ret1 = snprintf(result->string1, BUFFER_SIZE, "%f", random1);
    int ret2 = snprintf(result->string2, BUFFER_SIZE, "%f", random2);
    int ret3 = snprintf(result->string3, BUFFER_SIZE, "%f", random3);
    
    /* Validate snprintf results - check for errors and truncation */
    if (ret1 < 0 || ret1 >= BUFFER_SIZE ||
        ret2 < 0 || ret2 >= BUFFER_SIZE ||
        ret3 < 0 || ret3 >= BUFFER_SIZE) {
        /* Clear buffers on error for security */
        memset(result, 0, sizeof(RandomFloatStrings));
        return -1;
    }
    
    /* Explicitly ensure null termination (defensive programming) */
    result->string1[BUFFER_SIZE - 1] = '\\0';
    result->string2[BUFFER_SIZE - 1] = '\\0';
    result->string3[BUFFER_SIZE - 1] = '\\0';
    
    return 0;
}

int main(void) {
    RandomFloatStrings result;
    int status;
    
    /* Test case 1 */
    printf("Test case 1:\\n");
    status = generateRandomFloatStrings(&result);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to generate random floats\\n");
        return EXIT_FAILURE;
    }
    printf("Random float 1: %s\\n", result.string1);
    printf("Random float 2: %s\\n", result.string2);
    printf("Random float 3: %s\\n", result.string3);
    printf("\\n");
    
    /* Test case 2 */
    printf("Test case 2:\\n");
    status = generateRandomFloatStrings(&result);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to generate random floats\\n");
        return EXIT_FAILURE;
    }
    printf("Random float 1: %s\\n", result.string1);
    printf("Random float 2: %s\\n", result.string2);
    printf("Random float 3: %s\\n", result.string3);
    printf("\\n");
    
    /* Test case 3 */
    printf("Test case 3:\\n");
    status = generateRandomFloatStrings(&result);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to generate random floats\\n");
        return EXIT_FAILURE;
    }
    printf("Random float 1: %s\\n", result.string1);
    printf("Random float 2: %s\\n", result.string2);
    printf("Random float 3: %s\\n", result.string3);
    printf("\\n");
    
    /* Test case 4 */
    printf("Test case 4:\\n");
    status = generateRandomFloatStrings(&result);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to generate random floats\\n");
        return EXIT_FAILURE;
    }
    printf("Random float 1: %s\\n", result.string1);
    printf("Random float 2: %s\\n", result.string2);
    printf("Random float 3: %s\\n", result.string3);
    printf("\\n");
    
    /* Test case 5 */
    printf("Test case 5:\\n");
    status = generateRandomFloatStrings(&result);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to generate random floats\\n");
        return EXIT_FAILURE;
    }
    printf("Random float 1: %s\\n", result.string1);
    printf("Random float 2: %s\\n", result.string2);
    printf("Random float 3: %s\\n", result.string3);
    
    return EXIT_SUCCESS;
}
