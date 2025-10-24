
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

// Function to determine if sum of array elements is odd or even
// Returns "odd" or "even" as a string
// Treats empty array as [0] (even)
std::string oddOrEven(const std::vector<int>& arr) {
    // Security: Validate input size to prevent excessive memory operations
    // Maximum reasonable array size to prevent resource exhaustion
    const size_t MAX_ARRAY_SIZE = 10000000;
    if (arr.size() > MAX_ARRAY_SIZE) {
        throw std::invalid_argument("Array size exceeds maximum allowed");
    }
    
    // Handle empty array case: treat as [0] which is even
    if (arr.empty()) {
        return "even";
    }
    
    // Calculate sum with overflow protection
    // We only need to track odd/even, not the actual sum
    // This prevents integer overflow issues
    int64_t sum = 0;
    
    for (size_t i = 0; i < arr.size(); ++i) {
        // Bounds check already guaranteed by vector iterator
        // Check for potential overflow before adding
        if ((sum > 0 && arr[i] > INT64_MAX - sum) ||
            (sum < 0 && arr[i] < INT64_MIN - sum)) {
            throw std::overflow_error("Sum calculation would overflow");
        }
        sum += arr[i];
    }
    
    // Determine if sum is odd or even
    // Use modulo operation which is safe for negative numbers in C++
    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

int main() {
    // Test case 1: Array with single zero
    std::vector<int> test1 = {0};
    std::cout << "Test 1 [0]: " << oddOrEven(test1) << std::endl;
    
    // Test case 2: Array with odd sum
    std::vector<int> test2 = {0, 1, 4};
    std::cout << "Test 2 [0, 1, 4]: " << oddOrEven(test2) << std::endl;
    
    // Test case 3: Array with negative numbers, even sum
    std::vector<int> test3 = {0, -1, -5};
    std::cout << "Test 3 [0, -1, -5]: " << oddOrEven(test3) << std::endl;
    
    // Test case 4: Empty array (treated as [0])
    std::vector<int> test4 = {};
    std::cout << "Test 4 []: " << oddOrEven(test4) << std::endl;
    
    // Test case 5: Large positive and negative numbers
    std::vector<int> test5 = {1000000, -999999, 5};
    std::cout << "Test 5 [1000000, -999999, 5]: " << oddOrEven(test5) << std::endl;
    
    return 0;
}
