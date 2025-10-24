
#include <iostream>
#include <string>
#include <cmath>
#include <cstdint>
#include <climits>

// Function to find k such that sum of digits raised to consecutive powers equals k*n
// Returns k if found, -1 otherwise
long long digPow(long long n, int p) {
    // Input validation: n and p must be strictly positive
    if (n <= 0 || p <= 0) {
        return -1;
    }
    
    // Convert n to string to extract digits safely
    // This avoids repeated modulo operations and potential issues with negative numbers
    std::string numStr = std::to_string(n);
    
    // Bounds check: ensure we have at least one digit
    if (numStr.empty()) {
        return -1;
    }
    
    long long sum = 0;
    int currentPower = p;
    
    // Calculate sum of each digit raised to consecutive powers
    for (size_t i = 0; i < numStr.length(); i++) {
        // Extract digit and validate it's in range 0-9\n        int digit = numStr[i] - '0';
        if (digit < 0 || digit > 9) {
            return -1;
        }
        
        // Check for potential overflow before power calculation
        // If power is too large, result will overflow
        if (currentPower > 20) {  // pow(10, 20) exceeds long long range
            return -1;
        }
        
        // Calculate digit^currentPower safely
        double powerResult = std::pow(static_cast<double>(digit), static_cast<double>(currentPower));
        
        // Check if result is within long long range
        if (powerResult > static_cast<double>(LLONG_MAX) || powerResult < 0) {
            return -1;
        }
        
        long long powerValue = static_cast<long long>(powerResult);
        
        // Check for overflow before addition
        if (sum > LLONG_MAX - powerValue) {
            return -1;
        }
        
        sum += powerValue;
        currentPower++;
    }
    
    // Check if sum is divisible by n
    // Avoid division by zero (already checked n > 0)
    if (sum % n == 0) {
        long long k = sum / n;
        return k;
    }
    
    return -1;
}

int main() {
    // Test case 1: 89, p=1 -> expected 1
    std::cout << "Test 1: digPow(89, 1) = " << digPow(89, 1) << " (expected 1)" << std::endl;
    
    // Test case 2: 92, p=1 -> expected -1
    std::cout << "Test 2: digPow(92, 1) = " << digPow(92, 1) << " (expected -1)" << std::endl;
    
    // Test case 3: 695, p=2 -> expected 2
    std::cout << "Test 3: digPow(695, 2) = " << digPow(695, 2) << " (expected 2)" << std::endl;
    
    // Test case 4: 46288, p=3 -> expected 51
    std::cout << "Test 4: digPow(46288, 3) = " << digPow(46288, 3) << " (expected 51)" << std::endl;
    
    // Test case 5: edge case with 0
    std::cout << "Test 5: digPow(1, 1) = " << digPow(1, 1) << " (expected 1)" << std::endl;
    
    return 0;
}
