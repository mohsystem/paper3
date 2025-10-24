
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

// Function to sort positive numbers while keeping negative numbers in place
// Security: Input validation, bounds checking, no dynamic memory issues
std::vector<int> posNegSort(const std::vector<int>& arr) {
    // Handle empty array - return empty vector (fail-safe behavior)
    if (arr.empty()) {
        return std::vector<int>();
    }
    
    // Validate input size to prevent potential memory issues
    // Maximum reasonable array size check (prevents DoS via memory exhaustion)
    if (arr.size() > 1000000) {
        throw std::invalid_argument("Array size exceeds maximum allowed limit");
    }
    
    // Create a copy to avoid modifying input (const-correctness)
    std::vector<int> result = arr;
    
    // Extract positive numbers with bounds checking
    std::vector<int> positives;
    positives.reserve(arr.size()); // Pre-allocate to prevent multiple reallocations
    
    for (size_t i = 0; i < arr.size(); ++i) {
        // Only positive numbers (0 not included per spec)
        if (arr[i] > 0) {
            positives.push_back(arr[i]);
        }
    }
    
    // Sort positive numbers in ascending order using safe standard algorithm
    std::sort(positives.begin(), positives.end());
    
    // Place sorted positives back into their positions
    size_t posIndex = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] > 0) {
            // Bounds check before accessing positives vector
            if (posIndex >= positives.size()) {
                throw std::runtime_error("Index out of bounds in positives array");
            }
            result[i] = positives[posIndex];
            ++posIndex;
        }
    }
    
    return result;
}

int main() {
    // Test case 1: Mixed positive and negative numbers
    std::vector<int> test1 = {6, 3, -2, 5, -8, 2, -2};
    std::vector<int> result1 = posNegSort(test1);
    std::cout << "Test 1: ";
    for (int num : result1) std::cout << num << " ";
    std::cout << std::endl;
    
    // Test case 2: Multiple negatives at different positions
    std::vector<int> test2 = {6, 5, 4, -1, 3, 2, -1, 1};
    std::vector<int> result2 = posNegSort(test2);
    std::cout << "Test 2: ";
    for (int num : result2) std::cout << num << " ";
    std::cout << std::endl;
    
    // Test case 3: Mostly negative with one positive
    std::vector<int> test3 = {-5, -5, -5, -5, 7, -5};
    std::vector<int> result3 = posNegSort(test3);
    std::cout << "Test 3: ";
    for (int num : result3) std::cout << num << " ";
    std::cout << std::endl;
    
    // Test case 4: Empty array
    std::vector<int> test4 = {};
    std::vector<int> result4 = posNegSort(test4);
    std::cout << "Test 4: ";
    for (int num : result4) std::cout << num << " ";
    std::cout << "(empty)" << std::endl;
    
    // Test case 5: All positive numbers
    std::vector<int> test5 = {9, 3, 7, 1, 5};
    std::vector<int> result5 = posNegSort(test5);
    std::cout << "Test 5: ";
    for (int num : result5) std::cout << num << " ";
    std::cout << std::endl;
    
    return 0;
}
