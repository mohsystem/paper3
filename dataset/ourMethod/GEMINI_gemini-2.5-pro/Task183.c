#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Computes the largest product of a series of adjacent digits.
 *
 * @param input The sequence of digits to analyze.
 * @param span The number of digits in each series.
 * @param result A pointer to a long long to store the result.
 * @return 0 on success, -1 on failure.
 */
int largestProduct(const char* input, int span, long long* result) {
    if (input == NULL || result == NULL) {
        return -1; // Invalid pointers
    }

    size_t len = strlen(input);
    if (span < 0 || (size_t)span > len) {
        return -1; // Invalid span
    }

    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)input[i])) {
            return -1; // Invalid character in input
        }
    }

    if (span == 0) {
        *result = 1;
        return 0;
    }

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

    *result = maxProduct;
    return 0;
}

void run_test(const char* input, int span) {
    long long result;
    printf("Input: \"%.10s%s\", Span: %d, ", input, strlen(input) > 10 ? "..." : "", span);
    if (largestProduct(input, span, &result) == 0) {
        printf("Largest Product: %lld\n", result);
    } else {
        printf("Error: Invalid input.\n");
    }
}

int main() {
    // Test Case 1: Example from prompt
    run_test("63915", 3); // Expected: 162

    // Test Case 2: Contains a zero
    run_test("1027839564", 5); // Expected: 22680 (3*9*5*6*4)

    // Test Case 3: A longer number
    run_test("73167176531330624919225119674426574742355349194934", 6); // Expected: 23520

    // Test Case 4: Span is 0
    run_test("123", 0); // Expected: 1

    // Test Case 5: Span is 1
    run_test("18395", 1); // Expected: 9

    // Test Case 6: Invalid input (demonstrating error handling)
    run_test("123a45", 3); // Expected: Error

    return 0;
}