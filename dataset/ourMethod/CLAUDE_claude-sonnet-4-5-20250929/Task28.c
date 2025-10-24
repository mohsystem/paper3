
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Security: Define maximum limits to prevent memory exhaustion and buffer overflows
#define MAX_NAMES 10000
#define MAX_NAME_LENGTH 1000
#define MAX_OUTPUT_LENGTH 10000

// Function to generate "likes" display text
// Security: All parameters validated, all memory operations bounds-checked
// Returns dynamically allocated string that caller must free, or NULL on error
char* likes(const char* names[], size_t count) {
    // Security: Validate input array size to prevent DoS via memory exhaustion
    if (count > MAX_NAMES) {
        fprintf(stderr, "Error: Too many names in input array\\n");
        return NULL;
    }
    
    // Security: Validate each name pointer and length
    for (size_t i = 0; i < count; i++) {
        if (names[i] == NULL) {
            fprintf(stderr, "Error: NULL name pointer at index %zu\\n", i);
            return NULL;
        }
        size_t name_len = strnlen(names[i], MAX_NAME_LENGTH + 1);
        if (name_len == 0) {
            fprintf(stderr, "Error: Empty name at index %zu\\n", i);
            return NULL;
        }
        if (name_len > MAX_NAME_LENGTH) {
            fprintf(stderr, "Error: Name exceeds maximum length at index %zu\\n", i);
            return NULL;
        }
    }
    
    // Security: Allocate buffer with maximum expected size, initialized to zero
    char* result = (char*)calloc(MAX_OUTPUT_LENGTH, sizeof(char));
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Security: Use snprintf for all formatting to prevent buffer overflows
    int written = 0;
    
    switch (count) {
        case 0:
            // Security: Fixed string, no formatting needed
            written = snprintf(result, MAX_OUTPUT_LENGTH, "no one likes this");
            break;
        case 1:
            // Security: Validate buffer space before formatting
            written = snprintf(result, MAX_OUTPUT_LENGTH, "%s likes this", names[0]);
            break;
        case 2:
            // Security: snprintf prevents buffer overflow with size parameter
            written = snprintf(result, MAX_OUTPUT_LENGTH, "%s and %s like this", 
                             names[0], names[1]);
            break;
        case 3:
            // Security: Bounds-checked array access and safe formatting
            written = snprintf(result, MAX_OUTPUT_LENGTH, "%s, %s and %s like this", 
                             names[0], names[1], names[2]);
            break;
        default:
            // Security: Calculate others count with overflow check
            if (count < 2) {
                fprintf(stderr, "Error: Invalid count state\\n");
                free(result);
                return NULL;
            }
            size_t others = count - 2;
            // Security: Safe integer to string formatting with bounds check
            written = snprintf(result, MAX_OUTPUT_LENGTH, "%s, %s and %zu others like this", 
                             names[0], names[1], others);
            break;
    }
    
    // Security: Check if snprintf truncated output
    if (written < 0 || written >= MAX_OUTPUT_LENGTH) {
        fprintf(stderr, "Error: Output formatting failed or truncated\\n");
        free(result);
        return NULL;
    }
    
    // Security: Ensure null termination (snprintf guarantees this, but explicit check)
    result[MAX_OUTPUT_LENGTH - 1] = '\\0';
    
    return result;
}

int main(void) {
    char* result = NULL;
    
    // Test case 1: Empty array
    printf("Test 1: ");
    result = likes(NULL, 0);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 2: One name
    printf("Test 2: ");
    const char* names1[] = {"Peter"};
    result = likes(names1, 1);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 3: Two names
    printf("Test 3: ");
    const char* names2[] = {"Jacob", "Alex"};
    result = likes(names2, 2);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 4: Three names
    printf("Test 4: ");
    const char* names3[] = {"Max", "John", "Mark"};
    result = likes(names3, 3);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 5: Four or more names
    printf("Test 5: ");
    const char* names4[] = {"Alex", "Jacob", "Mark", "Max"};
    result = likes(names4, 4);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}
