#include <iostream>

// Function to check if a number is a Narcissistic Number
// A Narcissistic Number is a positive number which is the sum of its own digits,
// each raised to the power of the number of digits.
bool narcissistic(int value) {
    // According to the prompt, only valid positive non-zero integers will be passed.
    
    // Count digits using a mathematical approach.
    int temp_for_count = value;
    int num_digits = 0;
    // This loop handles all positive integers correctly.
    if (value == 0) { // Although prompt says positive, handle 0 for robustness.
        num_digits = 1;
    } else {
        while (temp_for_count > 0) {
            temp_for_count /= 10;
            num_digits++;
        }
    }
    
    // Use long long for the sum to prevent potential overflow.
    long long sum_of_powers = 0;
    int temp_for_sum = value;
    
    while (temp_for_sum > 0) {
        int digit = temp_for_sum % 10;
        
        // Calculate digit ^ num_digits.
        // A custom integer power loop is safer than std::pow with doubles.
        long long term = 1;
        for (int i = 0; i < num_digits; ++i) {
            term *= digit;
        }
        
        sum_of_powers += term;
        
        // Optimization: if the sum already exceeds the number, it can't be narcissistic.
        if (sum_of_powers > value) {
            return false;
        }
        
        temp_for_sum /= 10;
    }
    
    return sum_of_powers == value;
}

// Main function with 5 test cases
int main() {
    std::cout << "Testing narcissistic function:" << std::endl;
    
    // Test case 1: 7 (true)
    int test1 = 7;
    std::cout << "Is " << test1 << " narcissistic? " << (narcissistic(test1) ? "true" : "false") << std::endl;
    
    // Test case 2: 153 (true)
    int test2 = 153;
    std::cout << "Is " << test2 << " narcissistic? " << (narcissistic(test2) ? "true" : "false") << std::endl;
    
    // Test case 3: 1652 (false)
    int test3 = 1652;
    std::cout << "Is " << test3 << " narcissistic? " << (narcissistic(test3) ? "true" : "false") << std::endl;
    
    // Test case 4: 371 (true)
    int test4 = 371;
    std::cout << "Is " << test4 << " narcissistic? " << (narcissistic(test4) ? "true" : "false") << std::endl;
    
    // Test case 5: 9474 (true)
    int test5 = 9474; // 9^4 + 4^4 + 7^4 + 4^4 = 6561 + 256 + 2401 + 256 = 9474
    std::cout << "Is " << test5 << " narcissistic? " << (narcissistic(test5) ? "true" : "false") << std::endl;
    
    return 0;
}