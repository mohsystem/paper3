
#include <stdio.h>
#include <string.h>
#include <ctype.h>

long long largest_product(const char* input, int span) {
    // Handle edge cases
    if (input == NULL || span < 0) {
        return -1; // Error indicator
    }
    
    // Count digits
    int digit_count = 0;
    for (int i = 0; input[i] != '\\0'; i++) {
        if (isdigit(input[i])) {
            digit_count++;
        }
    }
    
    if (span > digit_count) {
        return -1; // Error indicator
    }
    
    if (span == 0) {
        return 1;
    }
    
    // Extract digits
    char digits[1000];
    int idx = 0;
    for (int i = 0; input[i] != '\\0' && idx < 999; i++) {
        if (isdigit(input[i])) {
            digits[idx++] = input[i];
        }
    }
    digits[idx] = '\\0';
    
    long long max_product = 0;
    
    // Iterate through all possible series
    for (int i = 0; i <= digit_count - span; i++) {
        long long product = 1;
        for (int j = i; j < i + span; j++) {
            product *= (digits[j] - '0');
        }
        if (product > max_product) {
            max_product = product;
        }
    }
    
    return max_product;
}

int main() {
    // Test case 1
    printf("Test 1: %lld\\n", largest_product("63915", 3)); // Expected: 162
    
    // Test case 2
    printf("Test 2: %lld\\n", largest_product("123456789", 4)); // Expected: 3024
    
    // Test case 3
    printf("Test 3: %lld\\n", largest_product("0123456789", 3)); // Expected: 504
    
    // Test case 4
    printf("Test 4: %lld\\n", largest_product("99999", 2)); // Expected: 81
    
    // Test case 5
    printf("Test 5: %lld\\n", largest_product("1234", 1)); // Expected: 4
    
    return 0;
}
