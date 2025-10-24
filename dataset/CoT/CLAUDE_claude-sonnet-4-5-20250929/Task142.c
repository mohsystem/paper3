
#include <stdio.h>
#include <stdlib.h>

// Function to find GCD using Euclidean algorithm
long long findGCD(long long a, long long b) {
    // Input validation - convert negative numbers to positive
    a = llabs(a);
    b = llabs(b);
    
    // Handle edge case where both numbers are 0
    if (a == 0 && b == 0) {
        fprintf(stderr, "Error: GCD is undefined for both numbers being zero\\n");
        return -1;
    }
    
    // Euclidean algorithm
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    
    return a;
}

int main() {
    long long result;
    
    // Test case 1: Normal positive numbers
    result = findGCD(48, 18);
    if (result != -1) {
        printf("Test 1 - GCD(48, 18): %lld\\n", result);
    }
    
    // Test case 2: One number is 0
    result = findGCD(0, 5);
    if (result != -1) {
        printf("Test 2 - GCD(0, 5): %lld\\n", result);
    }
    
    // Test case 3: Same numbers
    result = findGCD(100, 100);
    if (result != -1) {
        printf("Test 3 - GCD(100, 100): %lld\\n", result);
    }
    
    // Test case 4: Prime numbers (coprime)
    result = findGCD(17, 19);
    if (result != -1) {
        printf("Test 4 - GCD(17, 19): %lld\\n", result);
    }
    
    // Test case 5: Large numbers with negative input
    result = findGCD(-270, 192);
    if (result != -1) {
        printf("Test 5 - GCD(-270, 192): %lld\\n", result);
    }
    
    return 0;
}
