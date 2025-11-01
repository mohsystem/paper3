#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#define NUM_STRINGS 3
#define STRING_BUFFER_SIZE 50 // Sufficient for a standard float string representation

// Forward declaration
void free_string_array(char** arr, int size);

/**
 * @brief Generates three cryptographically secure random floats and converts them to strings.
 *        This version reads from /dev/urandom on POSIX-compliant systems. It uses
 *        snprintf for safe string formatting instead of the unsafe sprintf.
 * 
 * Note: The caller is responsible for freeing the returned array and each string within it
 *       by calling free_string_array().
 * 
 * @return A dynamically allocated array of 3 strings (char**), or NULL on failure.
 */
char** generateRandomFloatStrings() {
    int urandom_fd = -1;
    char** results = NULL;
    int i;

    urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd == -1) {
        perror("Failed to open /dev/urandom");
        return NULL;
    }

    results = (char**)malloc(NUM_STRINGS * sizeof(char*));
    if (results == NULL) {
        perror("Failed to allocate memory for results array");
        close(urandom_fd);
        return NULL;
    }
    for (i = 0; i < NUM_STRINGS; ++i) {
        results[i] = NULL;
    }

    for (i = 0; i < NUM_STRINGS; ++i) {
        uint32_t random_int;
        ssize_t bytes_read = read(urandom_fd, &random_int, sizeof(random_int));
        
        if (bytes_read != sizeof(random_int)) {
            fprintf(stderr, "Failed to read sufficient random bytes\n");
            free_string_array(results, NUM_STRINGS);
            close(urandom_fd);
            return NULL;
        }

        float random_float = (float)random_int / (float)UINT32_MAX;
        
        results[i] = (char*)malloc(STRING_BUFFER_SIZE);
        if (results[i] == NULL) {
            perror("Failed to allocate memory for string");
            free_string_array(results, NUM_STRINGS);
            close(urandom_fd);
            return NULL;
        }

        int written = snprintf(results[i], STRING_BUFFER_SIZE, "%f", random_float);
        if (written < 0 || (size_t)written >= STRING_BUFFER_SIZE) {
            fprintf(stderr, "snprintf failed or buffer was too small\n");
            free_string_array(results, NUM_STRINGS);
            close(urandom_fd);
            return NULL;
        }
    }

    close(urandom_fd);
    return results;
}

/**
 * @brief Frees a dynamically allocated array of strings.
 *
 * @param arr The array of strings to free.
 * @param size The number of strings in the array.
 */
void free_string_array(char** arr, int size) {
    if (arr == NULL) {
        return;
    }
    for (int i = 0; i < size; ++i) {
        free(arr[i]); // free() is safe to call on NULL
    }
    free(arr);
}

int main() {
    printf("Running 5 test cases...\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        char** randomStrings = generateRandomFloatStrings();
        if (randomStrings != NULL) {
            printf("  String 1: %s\n", randomStrings[0]);
            printf("  String 2: %s\n", randomStrings[1]);
            printf("  String 3: %s\n", randomStrings[2]);
            
            free_string_array(randomStrings, NUM_STRINGS);
        } else {
            fprintf(stderr, "  Failed to generate random strings.\n");
            return 1;
        }
        printf("\n");
    }

    return 0;
}