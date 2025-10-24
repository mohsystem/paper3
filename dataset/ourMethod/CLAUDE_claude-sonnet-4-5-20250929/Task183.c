
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>

// Security: Define error codes for different validation failures
#define ERROR_INVALID_SPAN -1
#define ERROR_EMPTY_INPUT -2
#define ERROR_SPAN_TOO_LARGE -3
#define ERROR_INVALID_CHAR -4
#define ERROR_OVERFLOW -5

// Function to find the largest product of adjacent digits with specified span
// Returns error code (negative) on failure, 0 or positive on success
int64_t largestProduct(const char* input, int span, int* error) {
    // Security: Initialize error code pointer
    if (error == NULL) {
        return ERROR_INVALID_SPAN;
    }
    *error = 0;
    
    // Security: Validate input pointer is not NULL
    if (input == NULL) {
        *error = ERROR_EMPTY_INPUT;
        return -1;
    }
    
    // Security: Validate span is non-negative to prevent negative indexing
    if (span < 0) {
        *error = ERROR_INVALID_SPAN;
        return -1;
    }
    
    // Security: Handle edge case where span is 0 (product is 1 by convention)
    if (span == 0) {
        return 1;
    }
    
    // Security: Get input length and validate
    size_t inputLen = strlen(input);
    if (inputLen == 0) {
        *error = ERROR_EMPTY_INPUT;
        return -1;
    }
    
    // Security: Check if span exceeds input length
    if ((size_t)span > inputLen) {
        *error = ERROR_SPAN_TOO_LARGE;
        return -1;
    }
    
    // Security: Validate that input contains only digits
    for (size_t i = 0; i < inputLen; i++) {
        if (!isdigit((unsigned char)input[i])) {
            *error = ERROR_INVALID_CHAR;
            return -1;
        }
    }
    
    int64_t maxProduct = 0;
    
    // Security: Use size_t for loop counter and bounds checking
    // Iterate through all possible series positions
    for (size_t i = 0; i <= inputLen - (size_t)span; i++) {
        int64_t product = 1;
        
        // Security: Calculate product with overflow check
        for (int j = 0; j < span; j++) {
            // Security: Bounds check before accessing input[i + j]
            size_t index = i + (size_t)j;
            if (index >= inputLen) {
                *error = ERROR_SPAN_TOO_LARGE;
                return -1;
            }
            
            int digit = input[index] - '0';
            
            // Security: Check for potential overflow before multiplication
            if (product > INT64_MAX / 10) {
                *error = ERROR_OVERFLOW;
                return -1;
            }
            
            product *= digit;
        }
        
        // Track maximum product
        if (product > maxProduct) {
            maxProduct = product;
        }
    }
    
    return maxProduct;
}

int main(void) {
    int error = 0;
    int64_t result = 0;
    
    // Test case 1: Example from problem
    printf("Test 1: ");
    error = 0;
    result = largestProduct("63915", 3, &error);
    if (error == 0) {
        printf("input=\\"63915\\", span=3 -> %lld\\n", (long long)result);
    } else {
        printf("Error code: %d\\n", error);
    }
    
    // Test case 2: Span of 0 (should return 1)
    printf("Test 2: ");
    error = 0;
    result = largestProduct("123", 0, &error);
    if (error == 0) {
        printf("input=\\"123\\", span=0 -> %lld\\n", (long long)result);
    } else {
        printf("Error code: %d\\n", error);
    }
    
    // Test case 3: Single digit span
    printf("Test 3: ");
    error = 0;
    result = largestProduct("987654321", 1, &error);
    if (error == 0) {
        printf("input=\\"987654321\\", span=1 -> %lld\\n", (long long)result);
    } else {
        printf("Error code: %d\\n", error);
    }
    
    // Test case 4: Span equals input length
    printf("Test 4: ");
    error = 0;
    result = largestProduct("12345", 5, &error);
    if (error == 0) {
        printf("input=\\"12345\\", span=5 -> %lld\\n", (long long)result);
    } else {
        printf("Error code: %d\\n", error);
    }
    
    // Test case 5: Invalid input - span larger than input
    printf("Test 5: ");
    error = 0;
    result = largestProduct("123", 5, &error);
    if (error == 0) {
        printf("input=\\"123\\", span=5 -> %lld\\n", (long long)result);
    } else {
        printf("Error code: %d (expected error)\\n", error);
    }
    
    return 0;
}
