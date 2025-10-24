
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

// Function to compute sum of integers between a and b (inclusive)
// Security: Input validation to prevent integer overflow in calculation
// Returns LLONG_MIN on error (overflow detection)
long long sumBetween(int a, int b) {
    // Handle equal case immediately to avoid unnecessary computation
    if (a == b) {
        return (long long)a;
    }
    
    // Ensure min <= max for consistent calculation
    int min = (a < b) ? a : b;
    int max = (a > b) ? a : b;
    
    // Validate range to prevent overflow in count calculation
    // Cast to long long to avoid overflow during subtraction
    long long range = (long long)max - (long long)min;
    
    // Check for overflow or invalid range
    if (range < 0) {
        fprintf(stderr, "Error: Range calculation overflow\\n");
        return LLONG_MIN;
    }
    
    // Bounds check: ensure count doesn't overflow
    if (range > LLONG_MAX - 1) {
        fprintf(stderr, "Error: Range too large\\n");
        return LLONG_MIN;
    }
    
    long long count = range + 1;
    
    // Use arithmetic series formula: sum = n * (first + last) / 2
    // Cast to long long to handle large intermediate values safely
    long long sum_operands = (long long)min + (long long)max;
    
    // Check for overflow in multiplication before performing it
    // For positive sum_operands
    if (sum_operands > 0 && count > LLONG_MAX / sum_operands) {
        fprintf(stderr, "Error: Sum calculation would overflow\\n");
        return LLONG_MIN;
    }
    // For negative sum_operands (division by negative can cause issues)
    if (sum_operands < 0 && count > LLONG_MAX / sum_operands) {
        fprintf(stderr, "Error: Sum calculation would overflow\\n");
        return LLONG_MIN;
    }
    
    long long sum = count * sum_operands / 2;
    
    return sum;
}

int main(void) {
    long long result;
    
    // Test case 1: (1, 0) --> 1
    result = sumBetween(1, 0);
    if (result != LLONG_MIN) {
        printf("Test 1: sumBetween(1, 0) = %lld\\n", result);
    }
    
    // Test case 2: (1, 2) --> 3
    result = sumBetween(1, 2);
    if (result != LLONG_MIN) {
        printf("Test 2: sumBetween(1, 2) = %lld\\n", result);
    }
    
    // Test case 3: (0, 1) --> 1
    result = sumBetween(0, 1);
    if (result != LLONG_MIN) {
        printf("Test 3: sumBetween(0, 1) = %lld\\n", result);
    }
    
    // Test case 4: (1, 1) --> 1
    result = sumBetween(1, 1);
    if (result != LLONG_MIN) {
        printf("Test 4: sumBetween(1, 1) = %lld\\n", result);
    }
    
    // Test case 5: (-1, 2) --> 2
    result = sumBetween(-1, 2);
    if (result != LLONG_MIN) {
        printf("Test 5: sumBetween(-1, 2) = %lld\\n", result);
    }
    
    return 0;
}
