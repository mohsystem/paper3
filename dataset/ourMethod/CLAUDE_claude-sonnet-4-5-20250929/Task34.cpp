
#include <iostream>
#include <cmath>
#include <vector>
#include <limits>

// Function to check if a number is narcissistic (Armstrong number)
// Security: Input validation for positive integers, overflow protection
bool isNarcissistic(int value) {
    // Security: Validate input is positive (per spec, only positive non-zero integers)
    if (value <= 0) {
        return false;
    }
    
    // Security: Check for potential overflow with INT_MAX
    if (value > INT_MAX) {
        return false;
    }
    
    // Extract digits and count them
    std::vector<int> digits;
    int temp = value;
    int numDigits = 0;
    
    // Security: Bounds check - prevent infinite loop
    while (temp > 0 && numDigits < 10) { // Max 10 digits for 32-bit int
        digits.push_back(temp % 10);
        temp /= 10;
        numDigits++;
    }
    
    // Security: Validate digit count is reasonable
    if (numDigits == 0 || numDigits > 10) {
        return false;
    }
    
    // Calculate sum of digits raised to power of digit count
    long long sum = 0; // Use long long to prevent overflow during calculation
    
    for (int digit : digits) {
        // Security: Validate digit is in valid range [0-9]
        if (digit < 0 || digit > 9) {
            return false;
        }
        
        // Security: Use pow carefully and check for overflow
        long long powered = static_cast<long long>(std::pow(digit, numDigits));
        
        // Security: Check for overflow before addition
        if (sum > LLONG_MAX - powered) {
            return false;
        }
        
        sum += powered;
    }
    
    // Compare sum with original value
    return (sum == static_cast<long long>(value));
}

int main() {
    // Test cases demonstrating narcissistic number detection
    std::cout << "Test 1 (153): " << (isNarcissistic(153) ? "true" : "false") << std::endl;
    std::cout << "Test 2 (1652): " << (isNarcissistic(1652) ? "true" : "false") << std::endl;
    std::cout << "Test 3 (9): " << (isNarcissistic(9) ? "true" : "false") << std::endl;
    std::cout << "Test 4 (9474): " << (isNarcissistic(9474) ? "true" : "false") << std::endl;
    std::cout << "Test 5 (1): " << (isNarcissistic(1) ? "true" : "false") << std::endl;
    
    return 0;
}
