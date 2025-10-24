
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

// Convert binary array to integer with full input validation
// Returns -1 on error, setting error message if provided
int binaryArrayToInt(const int* arr, size_t length, char* error_msg, size_t error_msg_size) {
    // Initialize error message buffer if provided
    if (error_msg != NULL && error_msg_size > 0) {
        error_msg[0] = '\\0';
    }
    
    // Input validation: check for NULL pointer
    if (arr == NULL) {
        if (error_msg != NULL && error_msg_size > 0) {
            snprintf(error_msg, error_msg_size, "Array pointer is NULL");
        }
        return -1;
    }
    
    // Input validation: check for empty array
    if (length == 0) {
        if (error_msg != NULL && error_msg_size > 0) {
            snprintf(error_msg, error_msg_size, "Array length is zero");
        }
        return -1;
    }
    
    // Security: prevent overflow by limiting array size to 31 bits
    // Use 31 instead of 32 to avoid sign bit issues with int
    if (length > 31) {
        if (error_msg != NULL && error_msg_size > 0) {
            snprintf(error_msg, error_msg_size, "Array too large - max 31 bits");
        }
        return -1;
    }
    
    int result = 0;
    
    // Process each element with validation
    for (size_t i = 0; i < length; i++) {
        // Security: validate each element is only 0 or 1
        if (arr[i] != 0 && arr[i] != 1) {
            if (error_msg != NULL && error_msg_size > 0) {
                snprintf(error_msg, error_msg_size, "Invalid value at index %zu: must be 0 or 1", i);
            }
            return -1;
        }
        
        // Security: check for overflow before shift
        // INT_MAX >> 1 is the maximum value that can be safely shifted left
        if (result > (INT_MAX >> 1)) {
            if (error_msg != NULL && error_msg_size > 0) {
                snprintf(error_msg, error_msg_size, "Integer overflow detected");
            }
            return -1;
        }
        
        // Shift left and add current bit (safe due to validation above)
        result = (result << 1) | arr[i];
    }
    
    return result;
}

int main(void) {
    char error_msg[256];
    int test_result = 0;
    
    // Initialize error message buffer
    memset(error_msg, 0, sizeof(error_msg));
    
    // Test case 1: [0, 0, 0, 1] => 1
    int test1[] = {0, 0, 0, 1};
    test_result = binaryArrayToInt(test1, 4, error_msg, sizeof(error_msg));
    if (test_result >= 0) {
        printf("Test 1: %d\\n", test_result);
    } else {
        fprintf(stderr, "Test 1 error: %s\\n", error_msg);
    }
    
    // Test case 2: [0, 0, 1, 0] => 2
    int test2[] = {0, 0, 1, 0};
    test_result = binaryArrayToInt(test2, 4, error_msg, sizeof(error_msg));
    if (test_result >= 0) {
        printf("Test 2: %d\\n", test_result);
    } else {
        fprintf(stderr, "Test 2 error: %s\\n", error_msg);
    }
    
    // Test case 3: [0, 1, 0, 1] => 5
    int test3[] = {0, 1, 0, 1};
    test_result = binaryArrayToInt(test3, 4, error_msg, sizeof(error_msg));
    if (test_result >= 0) {
        printf("Test 3: %d\\n", test_result);
    } else {
        fprintf(stderr, "Test 3 error: %s\\n", error_msg);
    }
    
    // Test case 4: [1, 0, 0, 1] => 9
    int test4[] = {1, 0, 0, 1};
    test_result = binaryArrayToInt(test4, 4, error_msg, sizeof(error_msg));
    if (test_result >= 0) {
        printf("Test 4: %d\\n", test_result);
    } else {
        fprintf(stderr, "Test 4 error: %s\\n", error_msg);
    }
    
    // Test case 5: [1, 1, 1, 1] => 15
    int test5[] = {1, 1, 1, 1};
    test_result = binaryArrayToInt(test5, 4, error_msg, sizeof(error_msg));
    if (test_result >= 0) {
        printf("Test 5: %d\\n", test_result);
    } else {
        fprintf(stderr, "Test 5 error: %s\\n", error_msg);
    }
    
    return 0;
}
