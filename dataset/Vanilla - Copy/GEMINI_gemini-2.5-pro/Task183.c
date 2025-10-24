#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

/**
 * @brief Computes the largest product of a series of adjacent digits of a specified span.
 * 
 * @param digits The sequence of digits to analyze.
 * @param span The number of digits in each series.
 * @return The largest product found, or -1 if an error occurs (e.g., invalid span, non-digit character).
 */
long long largestProduct(const char* digits, int span) {
    if (digits == NULL) {
        return -1; // Error: null string
    }
    
    size_t len = strlen(digits);

    if (span < 0 || (size_t)span > len) {
        return -1; // Error: invalid span
    }

    for (size_t i = 0; i < len; ++i) {
        if (!isdigit(digits[i])) {
            return -1; // Error: non-digit character found
        }
    }
    
    if (span == 0) {
        return 1; // The product of an empty set is conventionally 1.
    }

    long long maxProduct = 0;
    for (size_t i = 0; i <= len - span; ++i) {
        long long currentProduct = 1;
        for (int j = 0; j < span; ++j) {
            currentProduct *= (digits[i + j] - '0');
        }
        if (currentProduct > maxProduct) {
            maxProduct = currentProduct;
        }
    }

    return maxProduct;
}

int main() {
    long long result;

    // Test Case 1
    printf("Test Case 1: ('63915', 3)\n");
    result = largestProduct("63915", 3);
    printf("Result: %lld\n\n", result);

    // Test Case 2
    printf("Test Case 2: ('123456789', 4)\n");
    result = largestProduct("123456789", 4);
    printf("Result: %lld\n\n", result);

    // Test Case 3
    printf("Test Case 3: ('1110987', 3)\n");
    result = largestProduct("1110987", 3);
    printf("Result: %lld\n\n", result);

    // Test Case 4
    printf("Test Case 4: ('12345', 5)\n");
    result = largestProduct("12345", 5);
    printf("Result: %lld\n\n", result);

    // Test Case 5 (Invalid)
    printf("Test Case 5: ('123', 4)\n");
    result = largestProduct("123", 4);
    if (result == -1) {
        printf("Result: Invalid input\n\n");
    } else {
        printf("Result: %lld\n\n", result);
    }
    
    return 0;
}