#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

// Function to get a random unsigned int from a secure source if available.
// Falls back to pseudo-random if /dev/urandom is not readable.
unsigned int get_random_uint() {
    unsigned int val = 0;
    FILE *fp = fopen("/dev/urandom", "rb");
    if (fp != NULL) {
        // Read from the secure random source
        if (fread(&val, sizeof(val), 1, fp) == 1) {
            fclose(fp);
            return val;
        }
        fclose(fp);
    }
    // Fallback for non-POSIX or systems where /dev/urandom is unavailable.
    // Note: This fallback is not cryptographically secure.
    return (unsigned int)rand();
}

// Generate a random float between 0.0 and 1000.0
float generate_random_float() {
    return ((float)get_random_uint() / (float)UINT_MAX) * 1000.0f;
}

// Converts a float to a dynamically allocated string.
// The caller is responsible for freeing the returned string.
char* floatToString(float f) {
    // A buffer of 64 chars is sufficient for any standard float representation.
    char* buffer = (char*)malloc(64 * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate memory for string");
        return NULL;
    }

    // Use snprintf for safe string formatting.
    int ret = snprintf(buffer, 64, "%f", f);
    if (ret < 0 || ret >= 64) {
        fprintf(stderr, "snprintf encoding error or buffer too small.\n");
        free(buffer);
        return NULL;
    }
    
    return buffer;
}


// Generates three random floats and stores their string representations
// in the provided output pointers. Memory is allocated and must be freed by the caller.
// Returns 0 on success, -1 on failure.
int generateThreeRandomFloatStrings(char** str_a, char** str_b, char** str_c) {
    if (!str_a || !str_b || !str_c) {
        return -1;
    }

    *str_a = floatToString(generate_random_float());
    *str_b = floatToString(generate_random_float());
    *str_c = floatToString(generate_random_float());
    
    if (!(*str_a) || !(*str_b) || !(*str_c)) {
        // Cleanup in case of partial failure
        free(*str_a);
        free(*str_b);
        free(*str_c);
        *str_a = NULL;
        *str_b = NULL;
        *str_c = NULL;
        return -1;
    }
    return 0;
}


int main() {
    // Seed the fallback random number generator once.
    // This is only used if /dev/urandom is not available.
    srand((unsigned int)time(NULL));

    printf("Running 5 test cases...\n");
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        
        char *str_a = NULL;
        char *str_b = NULL;
        char *str_c = NULL;

        if (generateThreeRandomFloatStrings(&str_a, &str_b, &str_c) == 0) {
            printf("str_a: %s\n", str_a);
            printf("str_b: %s\n", str_b);
            printf("str_c: %s\n", str_c);
        } else {
            fprintf(stderr, "Failed to generate float strings.\n");
        }

        // IMPORTANT: Free the allocated memory
        free(str_a);
        free(str_b);
        free(str_c);
    }

    return 0;
}