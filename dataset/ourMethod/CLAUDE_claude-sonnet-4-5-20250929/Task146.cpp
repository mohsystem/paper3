
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <limits>

// Function to find the missing number in an array of unique integers from 1 to n
// Security: Uses mathematical formula (sum of first n natural numbers) to avoid overflow
// Validates input array size and checks for integer overflow
int findMissingNumber(const std::vector<int>& arr) {
    // Rule#3: Validate input - array must not be empty
    if (arr.empty()) {
        throw std::invalid_argument("Array cannot be empty");
    }
    
    // Rule#3: Validate input - array size must be reasonable to prevent overflow
    // n+1 is the original size (including missing number), so arr.size() = n
    size_t n = arr.size() + 1;
    
    // Rule#3: Check for overflow: n should not exceed reasonable bounds
    // Maximum safe value to avoid overflow in sum calculation
    if (n > 100000) {
        throw std::invalid_argument("Array size too large");
    }
    
    // Calculate expected sum using formula: n * (n + 1) / 2
    // Rule#4: Check for potential integer overflow before calculation
    if (n > static_cast<size_t>(std::numeric_limits<long long>::max() / (n + 1))) {
        throw std::overflow_error("Sum calculation would overflow");
    }
    
    long long expectedSum = static_cast<long long>(n) * (n + 1) / 2;
    long long actualSum = 0;
    
    // Rule#3: Validate each element and calculate actual sum
    for (size_t i = 0; i < arr.size(); ++i) {
        // Rule#3: Validate that each number is within valid range [1, n]
        if (arr[i] < 1 || arr[i] > static_cast<int>(n)) {
            throw std::invalid_argument("Array contains invalid number outside range [1, n]");
        }
        
        // Rule#4: Check for overflow before addition
        if (actualSum > std::numeric_limits<long long>::max() - arr[i]) {
            throw std::overflow_error("Actual sum calculation would overflow");
        }
        
        actualSum += arr[i];
    }
    
    // The missing number is the difference
    long long missing = expectedSum - actualSum;
    
    // Rule#3: Validate result is within valid range
    if (missing < 1 || missing > static_cast<long long>(n)) {
        throw std::logic_error("Calculated missing number is invalid");
    }
    
    return static_cast<int>(missing);
}

int main() {
    // Test case 1: Missing number is 5
    try {
        std::vector<int> test1 = {1, 2, 3, 4, 6, 7, 8, 9, 10};
        int result1 = findMissingNumber(test1);
        std::cout << "Test 1 - Missing number: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: Missing number is 1
    try {
        std::vector<int> test2 = {2, 3, 4, 5};
        int result2 = findMissingNumber(test2);
        std::cout << "Test 2 - Missing number: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: Missing number is 10 (last element)
    try {
        std::vector<int> test3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        int result3 = findMissingNumber(test3);
        std::cout << "Test 3 - Missing number: " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: Single element array (missing 2)
    try {
        std::vector<int> test4 = {1};
        int result4 = findMissingNumber(test4);
        std::cout << "Test 4 - Missing number: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: Missing number is 3 (middle element)
    try {
        std::vector<int> test5 = {1, 2, 4, 5, 6};
        int result5 = findMissingNumber(test5);
        std::cout << "Test 5 - Missing number: " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
