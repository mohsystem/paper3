
#include <iostream>
#include <stdexcept>
#include <limits>

// Function to compute sum of integers between a and b (inclusive)
// Security: Input validation to prevent integer overflow in calculation
long long sumBetween(int a, int b) {
    // Handle equal case immediately to avoid unnecessary computation
    if (a == b) {
        return a;
    }
    
    // Ensure min <= max for consistent calculation
    int min = (a < b) ? a : b;
    int max = (a > b) ? a : b;
    
    // Validate range to prevent overflow in count calculation
    // max - min must fit in a long long to avoid overflow
    long long range = static_cast<long long>(max) - static_cast<long long>(min);
    if (range < 0 || range > LLONG_MAX - 1) {
        throw std::overflow_error("Range too large");
    }
    
    long long count = range + 1;
    
    // Use arithmetic series formula: sum = n * (first + last) / 2
    // Check for potential overflow before multiplication
    // Cast to long long to handle large intermediate values safely
    long long sum_check = static_cast<long long>(min) + static_cast<long long>(max);
    
    // Validate multiplication won't overflow
    if (count > 0 && sum_check > 0 && count > LLONG_MAX / sum_check) {
        throw std::overflow_error("Sum calculation would overflow");
    }
    if (count > 0 && sum_check < 0 && count > LLONG_MAX / sum_check) {
        throw std::overflow_error("Sum calculation would overflow");
    }
    
    long long sum = count * sum_check / 2;
    
    return sum;
}

int main() {
    // Test case 1: (1, 0) --> 1
    try {
        std::cout << "Test 1: sumBetween(1, 0) = " << sumBetween(1, 0) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: (1, 2) --> 3
    try {
        std::cout << "Test 2: sumBetween(1, 2) = " << sumBetween(1, 2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: (0, 1) --> 1
    try {
        std::cout << "Test 3: sumBetween(0, 1) = " << sumBetween(0, 1) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: (1, 1) --> 1
    try {
        std::cout << "Test 4: sumBetween(1, 1) = " << sumBetween(1, 1) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: (-1, 2) --> 2
    try {
        std::cout << "Test 5: sumBetween(-1, 2) = " << sumBetween(-1, 2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
