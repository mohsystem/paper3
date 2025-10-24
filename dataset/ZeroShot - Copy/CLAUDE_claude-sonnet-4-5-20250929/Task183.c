
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

long long largestProduct(const char* input, int span) {
    // Validate inputs
    if (input == NULL || strlen(input) == 0) {
        fprintf(stderr, "Error: Input cannot be NULL or empty\\n");
        return -1;
    }
    
    int inputLen = strlen(input);
    
    if (span < 0) {
        fprintf(stderr, "Error: Span cannot be negative\\n");
        return -1;
    }
    
    if (span == 0) {
        return 1;
    }
    
    if (span > inputLen) {
        fprintf(stderr, "Error: Span cannot be larger than input length\\n");
        return -1;
    }
    
    // Validate that input contains only digits
    for (int i = 0; i < inputLen; i++) {
        if (!isdigit(input[i])) {
            fprintf(stderr, "Error: Input must contain only digits\\n");
            return -1;
        }
    }
    
    long long maxProduct = 0;
    
    // Iterate through all possible series
    for (int i = 0; i <= inputLen - span; i++) {
        long long product = 1;
        
        // Calculate product of current series
        for (int j = i; j < i + span; j++) {
            product *= (input[j] - '0');
        }
        
        // Update max product if current is larger
        if (product > maxProduct) {
            maxProduct = product;
        }
    }
    
    return maxProduct;
}

int main() {
    // Test case 1: Example from prompt
    printf("Test 1: %lld\\n", largestProduct("63915", 3)); // Expected: 162
    
    // Test case 2: Span of 1
    printf("Test 2: %lld\\n", largestProduct("63915", 1)); // Expected: 9
    
    // Test case 3: Span equals input length
    printf("Test 3: %lld\\n", largestProduct("123", 3)); // Expected: 6
    
    // Test case 4: With zeros
    printf("Test 4: %lld\\n", largestProduct("1203", 2)); // Expected: 6
    
    // Test case 5: Larger number
    printf("Test 5: %lld\\n", largestProduct("73167176531330624919225119674426574742355349194934", 6)); // Expected: 23520
    
    return 0;
}
