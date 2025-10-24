
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

// Security: Define maximum array size to prevent resource exhaustion
#define MAX_ARRAY_SIZE 10000000

// Function to determine if sum of array elements is odd or even
// Returns dynamically allocated string "odd" or "even"
// Caller must free the returned string
// Returns NULL on error
char* oddOrEven(const int* arr, size_t length) {
    char* result = NULL;
    
    // Security: Validate input pointer if array is non-empty
    if (length > 0 && arr == NULL) {
        fprintf(stderr, "Error: NULL array pointer with non-zero length\\n");
        return NULL;
    }
    
    // Security: Validate array size to prevent resource exhaustion
    if (length > MAX_ARRAY_SIZE) {
        fprintf(stderr, "Error: Array size exceeds maximum allowed\\n");
        return NULL;
    }
    
    // Handle empty array case: treat as [0] which is even
    if (length == 0) {
        result = (char*)malloc(5 * sizeof(char)); // "even" + null terminator
        if (result == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\\n");
            return NULL;
        }
        strncpy(result, "even", 5);
        result[4] = '\\0'; // Ensure null termination
        return result;
    }
    
    // Calculate sum with overflow protection
    // Only track odd/even to prevent overflow issues
    int64_t sum = 0;
    
    for (size_t i = 0; i < length; i++) {
        // Security: Check for integer overflow before addition
        if ((sum > 0 && arr[i] > INT64_MAX - sum) ||
            (sum < 0 && arr[i] < INT64_MIN - sum)) {
            fprintf(stderr, "Error: Sum calculation would overflow\\n");
            return NULL;
        }
        sum += arr[i];
    }
    
    // Determine if sum is odd or even
    if (sum % 2 == 0) {
        result = (char*)malloc(5 * sizeof(char)); // "even" + null terminator
        if (result == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\\n");
            return NULL;
        }
        strncpy(result, "even", 5);
        result[4] = '\\0'; // Ensure null termination
    } else {
        result = (char*)malloc(4 * sizeof(char)); // "odd" + null terminator
        if (result == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\\n");
            return NULL;
        }
        strncpy(result, "odd", 4);
        result[3] = '\\0'; // Ensure null termination
    }
    
    return result;
}

int main() {
    char* result = NULL;
    
    // Test case 1: Array with single zero
    int test1[] = {0};
    result = oddOrEven(test1, 1);
    if (result != NULL) {
        printf("Test 1 [0]: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 2: Array with odd sum
    int test2[] = {0, 1, 4};
    result = oddOrEven(test2, 3);
    if (result != NULL) {
        printf("Test 2 [0, 1, 4]: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 3: Array with negative numbers, even sum
    int test3[] = {0, -1, -5};
    result = oddOrEven(test3, 3);
    if (result != NULL) {
        printf("Test 3 [0, -1, -5]: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 4: Empty array (treated as [0])
    result = oddOrEven(NULL, 0);
    if (result != NULL) {
        printf("Test 4 []: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    // Test case 5: Large positive and negative numbers
    int test5[] = {1000000, -999999, 5};
    result = oddOrEven(test5, 3);
    if (result != NULL) {
        printf("Test 5 [1000000, -999999, 5]: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}
