
#include <stdio.h>
#include <stdlib.h>

int persistence(long long num) {
    // Input validation: ensure num is positive
    if (num < 0) {
        fprintf(stderr, "Error: Number must be positive\\n");
        exit(EXIT_FAILURE);
    }
    
    // Base case: single digit
    if (num < 10) {
        return 0;
    }
    
    int count = 0;
    
    // Continue until we reach a single digit
    while (num >= 10) {
        long long product = 1;
        
        // Multiply all digits
        while (num > 0) {
            product *= num % 10;
            num /= 10;
        }
        
        num = product;
        count++;
    }
    
    return count;
}

int main() {
    // Test cases
    printf("Test 1: persistence(39) = %d (expected: 3)\\n", persistence(39));
    printf("Test 2: persistence(999) = %d (expected: 4)\\n", persistence(999));
    printf("Test 3: persistence(4) = %d (expected: 0)\\n", persistence(4));
    printf("Test 4: persistence(25) = %d (expected: 2)\\n", persistence(25));
    printf("Test 5: persistence(10) = %d (expected: 1)\\n", persistence(10));
    
    return 0;
}
