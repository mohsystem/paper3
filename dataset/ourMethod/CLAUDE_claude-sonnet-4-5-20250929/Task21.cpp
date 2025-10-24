
#include <vector>
#include <limits>
#include <iostream>
#include <stdexcept>

// Function to remove the smallest value from array (first occurrence)
// Returns a new vector without mutating the original
// Security: validates input size, uses bounds checking, prevents integer overflow
std::vector<int> removeSmallest(const std::vector<int>& numbers) {
    // Validate input - empty array returns empty array as per spec
    if (numbers.empty()) {
        return std::vector<int>();
    }
    
    // Security: check size to prevent potential issues with very large arrays
    // Limit to reasonable size to prevent excessive memory allocation
    if (numbers.size() > 1000000) {
        throw std::invalid_argument("Input array too large");
    }
    
    // Find the minimum value and its index
    // Use size_t for index to prevent signed/unsigned issues
    int minValue = std::numeric_limits<int>::max();
    size_t minIndex = 0;
    
    // Security: use at() for bounds checking or validate index manually
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (numbers[i] < minValue) {
            minValue = numbers[i];
            minIndex = i;
        }
    }
    
    // Create result vector without the smallest element
    // Security: reserve exact size needed to prevent repeated allocations
    std::vector<int> result;
    result.reserve(numbers.size() - 1);
    
    // Copy all elements except the one at minIndex
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (i != minIndex) {
            result.push_back(numbers[i]);
        }
    }
    
    return result;
}

int main() {
    // Test case 1: [1,2,3,4,5] -> [2,3,4,5]
    std::vector<int> test1 = {1, 2, 3, 4, 5};
    std::vector<int> result1 = removeSmallest(test1);
    std::cout << "Test 1: [";
    for (size_t i = 0; i < result1.size(); ++i) {
        std::cout << result1[i];
        if (i < result1.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 2: [5,3,2,1,4] -> [5,3,2,4]
    std::vector<int> test2 = {5, 3, 2, 1, 4};
    std::vector<int> result2 = removeSmallest(test2);
    std::cout << "Test 2: [";
    for (size_t i = 0; i < result2.size(); ++i) {
        std::cout << result2[i];
        if (i < result2.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 3: [2,2,1,2,1] -> [2,2,2,1]
    std::vector<int> test3 = {2, 2, 1, 2, 1};
    std::vector<int> result3 = removeSmallest(test3);
    std::cout << "Test 3: [";
    for (size_t i = 0; i < result3.size(); ++i) {
        std::cout << result3[i];
        if (i < result3.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 4: [] -> []
    std::vector<int> test4 = {};
    std::vector<int> result4 = removeSmallest(test4);
    std::cout << "Test 4: [";
    for (size_t i = 0; i < result4.size(); ++i) {
        std::cout << result4[i];
        if (i < result4.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    // Test case 5: [7] -> []
    std::vector<int> test5 = {7};
    std::vector<int> result5 = removeSmallest(test5);
    std::cout << "Test 5: [";
    for (size_t i = 0; i < result5.size(); ++i) {
        std::cout << result5[i];
        if (i < result5.size() - 1) std::cout << ",";
    }
    std::cout << "]" << std::endl;
    
    return 0;
}
