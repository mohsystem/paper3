
#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <limits>

// Function to find the integer that appears an odd number of times
// Uses XOR property: a^a = 0, a^0 = a, XOR is commutative and associative
// All numbers appearing even times cancel out, leaving only the odd occurrence
int findOddOccurrence(const std::vector<int>& arr) {
    // Validate input: array must not be empty
    if (arr.empty()) {
        throw std::invalid_argument("Array cannot be empty");
    }
    
    // Validate input size to prevent potential overflow issues
    if (arr.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Array size exceeds safe limits");
    }
    
    // XOR all elements: numbers appearing even times cancel out
    // This is constant time comparison safe and efficient O(n) time, O(1) space
    int result = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        result ^= arr[i];
    }
    
    return result;
}

int main() {
    // Test case 1: Single element
    try {
        std::vector<int> test1 = {7};
        int result1 = findOddOccurrence(test1);
        std::cout << "Test 1: " << result1 << " (expected 7)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: Single zero
    try {
        std::vector<int> test2 = {0};
        int result2 = findOddOccurrence(test2);
        std::cout << "Test 2: " << result2 << " (expected 0)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: Simple case with pairs
    try {
        std::vector<int> test3 = {1, 1, 2};
        int result3 = findOddOccurrence(test3);
        std::cout << "Test 3: " << result3 << " (expected 2)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: Multiple occurrences
    try {
        std::vector<int> test4 = {0, 1, 0, 1, 0};
        int result4 = findOddOccurrence(test4);
        std::cout << "Test 4: " << result4 << " (expected 0)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: Complex case
    try {
        std::vector<int> test5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
        int result5 = findOddOccurrence(test5);
        std::cout << "Test 5: " << result5 << " (expected 4)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
