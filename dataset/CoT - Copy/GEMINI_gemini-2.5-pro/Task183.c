#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Computes the largest product of a series of adjacent digits of a specified span.
 *
 * @param input The sequence of digits to analyze.
 * @param span  The number of digits in each series.
 * @param result_ptr Pointer to a long long to store the result.
 * @return true on success, false on failure (invalid input).
 */
bool largestProduct(const char* input, int span, long long* result_ptr) {
    // 1. Input Validation
    if (input == NULL || result_ptr == NULL) {
        return false; // Null pointers
    }
    size_t len = strlen(input);
    if (span < 0) {
        return false; // Negative span
    }
    if ((size_t)span > len) {
        return false; // Span too large
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)input[i])) {
            return false; // Non-digit character
        }
    }

    // 2. Handle edge case: span is 0, the product of an empty set is 1.
    if (span == 0) {
        *result_ptr = 1;
        return true;
    }

    // 3. Main Algorithm
    long long maxProduct = 0;

    for (size_t i = 0; i <= len - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            currentProduct *= (input[i + j] - '0');
        }
        if (currentProduct > maxProduct) {
            maxProduct = currentProduct;
        }
    }

    *result_ptr = maxProduct;
    return true;
}

int main() {
    // Test Cases
    const char* inputs[] = {
        "63915",
        "1027839564",
        "12345",
        "12345",
        "73167176531330624919225119674426574742355349194934"
    };
    int spans[] = {3, 4, 5, 0, 6};
    long long expectedOutputs[] = {162, 5832, 120, 1, 23520};
    int num_tests = sizeof(spans) / sizeof(spans[0]);

    for (int i = 0; i < num_tests; ++i) {
        long long result;
        bool success = largestProduct(inputs[i], spans[i], &result);

        printf("Test Case %d:\n", i + 1);
        printf("Input: \"%s\", Span: %d\n", inputs[i], spans[i]);
        if (success) {
            printf("Result: %lld\n", result);
            printf("Expected: %lld\n", expectedOutputs[i]);
            printf("Status: %s\n", (result == expectedOutputs[i] ? "Passed" : "Failed"));
        } else {
            printf("Function failed due to invalid input.\n");
        }
        printf("--------------------\n");
    }
    
    // Test invalid cases
    long long invalid_result;
    bool success;

    success = largestProduct("123", 4, &invalid_result);
    printf("Test Case 6 (Invalid Span): %s\n", success ? "Failed" : "Passed - correctly identified error.");

    success = largestProduct("1a2", 2, &invalid_result);
    printf("Test Case 7 (Invalid Chars): %s\n", success ? "Failed" : "Passed - correctly identified error.");

    return 0;
}