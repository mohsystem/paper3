
#include <stdio.h>
#include <limits.h>
#include <errno.h>

// Function to calculate multiplicative persistence
// Returns the number of times digits must be multiplied until a single digit remains
// Returns -1 on error (invalid input)
int persistence(long long num) {
    // Validate input: must be non-negative as per problem statement
    if (num < 0) {
        errno = EINVAL;
        return -1;
    }
    
    // Prevent overflow: limit input to reasonable range
    // Maximum safe value that won't cause overflow during digit multiplication
    if (num > 999999999999999LL) {
        errno = ERANGE;
        return -1;
    }
    
    int count = 0;
    
    // Continue until we reach a single digit (0-9)
    while (num >= 10) {
        long long product = 1;
        
        // Multiply all digits together
        while (num > 0) {
            long long digit = num % 10;
            
            // Check for potential overflow before multiplication
            // This prevents integer overflow vulnerability
            if (digit != 0 && product > LLONG_MAX / digit) {
                errno = ERANGE;
                return -1;
            }
            
            product *= digit;
            num /= 10;
        }
        
        num = product;
        count++;
        
        // Sanity check: prevent infinite loop (though mathematically impossible)
        // This adds defense-in-depth against unexpected behavior
        if (count > 1000) {
            errno = ERANGE;
            return -1;
        }
    }
    
    return count;
}

int main(void) {
    int result;
    
    // Test case 1: 39 -> 3
    result = persistence(39);
    if (result >= 0) {
        printf("Test 1: persistence(39) = %d (expected: 3)\\n", result);
    } else {
        printf("Test 1: Error occurred\\n");
    }
    
    // Test case 2: 999 -> 4
    result = persistence(999);
    if (result >= 0) {
        printf("Test 2: persistence(999) = %d (expected: 4)\\n", result);
    } else {
        printf("Test 2: Error occurred\\n");
    }
    
    // Test case 3: 4 -> 0
    result = persistence(4);
    if (result >= 0) {
        printf("Test 3: persistence(4) = %d (expected: 0)\\n", result);
    } else {
        printf("Test 3: Error occurred\\n");
    }
    
    // Test case 4: 25 -> 2
    result = persistence(25);
    if (result >= 0) {
        printf("Test 4: persistence(25) = %d (expected: 2)\\n", result);
    } else {
        printf("Test 4: Error occurred\\n");
    }
    
    // Test case 5: 77 -> 4
    result = persistence(77);
    if (result >= 0) {
        printf("Test 5: persistence(77) = %d (expected: 4)\\n", result);
    } else {
        printf("Test 5: Error occurred\\n");
    }
    
    return 0;
}
