#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

/**
 * Computes the largest product of a series of adjacent digits.
 *
 * @param input The sequence of digits as a null-terminated string.
 * @param span The number of digits in each series.
 * @param result A pointer to a long long to store the result.
 * @return 0 on success, -1 on failure. An error message is printed to stderr on failure.
 */
int largestProduct(const char* input, int span, long long* result) {
    // 1. Input Validation
    if (input == NULL || result == NULL) {
        fprintf(stderr, "Error: Null pointer provided as input.\n");
        return -1;
    }
    if (span < 0) {
        fprintf(stderr, "Error: Span cannot be negative.\n");
        return -1;
    }
    
    size_t len = strlen(input);

    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)input[i])) {
            fprintf(stderr, "Error: Input string must contain only digits.\n");
            return -1;
        }
    }

    if (span > len) {
        fprintf(stderr, "Error: Span cannot be greater than the input string length.\n");
        return -1;
    }

    // 2. Edge Case
    if (span == 0) {
        *result = 1;
        return 0;
    }

    // 3. Calculation
    long long maxProduct = 0;
    for (size_t i = 0; i <= len - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            // Convert character to its numeric value and multiply
            currentProduct *= (input[i + j] - '0');
        }
        if (currentProduct > maxProduct) {
            maxProduct = currentProduct;
        }
    }
    
    *result = maxProduct;
    return 0;
}

int main() {
    // 5 Test Cases
    long long result;

    // Test Case 1: Example from prompt
    if (largestProduct("63915", 3, &result) == 0) {
        printf("%lld\n", result);
    }

    // Test Case 2: A sequence containing zero
    if (largestProduct("1234560789", 5, &result) == 0) {
        printf("%lld\n", result);
    }

    // Test Case 3: Span of 1
    if (largestProduct("987654321", 1, &result) == 0) {
        printf("%lld\n", result);
    }

    // Test Case 4: Span equals the length of the input
    if (largestProduct("12345", 5, &result) == 0) {
        printf("%lld\n", result);
    }

    // Test Case 5: Span of 0
    if (largestProduct("111111", 0, &result) == 0) {
        printf("%lld\n", result);
    }

    return 0;
}