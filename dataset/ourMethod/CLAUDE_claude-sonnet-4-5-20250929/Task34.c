
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

// Function to check if a number is narcissistic (Armstrong number)
// Security: Input validation, bounds checking, overflow protection
bool isNarcissistic(int value) {
    // Security: Validate input is positive (per spec, only positive non-zero integers)
    if (value <= 0) {
        return false;
    }
    
    // Security: Check upper bound to prevent overflow
    if (value > INT_MAX) {
        return false;
    }
    
    // Extract digits and count them
    int digits[10]; // Security: Fixed size array to prevent overflow, max 10 digits for int
    int numDigits = 0;
    int temp = value;
    
    // Security: Initialize array to zero
    for (int i = 0; i < 10; i++) {
        digits[i] = 0;
    }
    
    // Security: Bounds check on array index
    while (temp > 0 && numDigits < 10) {
        digits[numDigits] = temp % 10;
        temp /= 10;
        numDigits++;
    }
    
    // Security: Validate digit count
    if (numDigits == 0 || numDigits > 10) {
        return false;
    }
    
    // Calculate sum of digits raised to power of digit count
    long long sum = 0; // Use long long to prevent overflow during calculation
    
    for (int i = 0; i < numDigits; i++) {
        int digit = digits[i];
        
        // Security: Validate digit is in valid range [0-9]
        if (digit < 0 || digit > 9) {
            return false;
        }
        
        // Security: Use pow carefully and cast to long long
        long long powered = (long long)pow((double)digit, (double)numDigits);
        
        // Security: Check for overflow before addition
        if (sum > LLONG_MAX - powered) {
            return false;
        }
        
        sum += powered;
    }
    
    // Compare sum with original value
    return (sum == (long long)value);
}

int main(void) {
    // Test cases demonstrating narcissistic number detection
    printf("Test 1 (153): %s\\n", isNarcissistic(153) ? "true" : "false");
    printf("Test 2 (1652): %s\\n", isNarcissistic(1652) ? "true" : "false");
    printf("Test 3 (9): %s\\n", isNarcissistic(9) ? "true" : "false");
    printf("Test 4 (9474): %s\\n", isNarcissistic(9474) ? "true" : "false");
    printf("Test 5 (1): %s\\n", isNarcissistic(1) ? "true" : "false");
    
    return 0;
}
