
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstdint>

// Sorts an array of integers in ascending order using std::sort
// Parameters: vec - vector of integers to sort
// Returns: sorted vector (creates a copy to avoid modifying input directly)
std::vector<int> sortArrayAscending(const std::vector<int>& vec) {
    // Input validation: check if vector is too large to prevent resource exhaustion
    if (vec.size() > 1000000) {
        throw std::invalid_argument("Input array too large");
    }
    
    // Create a copy to avoid modifying the original
    std::vector<int> result = vec;
    
    // Use standard library sort (guaranteed O(n log n) complexity)
    // This is secure and well-tested implementation
    std::sort(result.begin(), result.end());
    
    return result;
}

int main() {
    // Test case 1: Normal array
    std::vector<int> test1 = {64, 34, 25, 12, 22, 11, 90};
    std::vector<int> result1 = sortArrayAscending(test1);
    std::cout << "Test 1: ";
    for (int num : result1) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Test case 2: Already sorted array
    std::vector<int> test2 = {1, 2, 3, 4, 5};
    std::vector<int> result2 = sortArrayAscending(test2);
    std::cout << "Test 2: ";
    for (int num : result2) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Test case 3: Reverse sorted array
    std::vector<int> test3 = {9, 7, 5, 3, 1};
    std::vector<int> result3 = sortArrayAscending(test3);
    std::cout << "Test 3: ";
    for (int num : result3) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Test case 4: Array with negative numbers
    std::vector<int> test4 = {-5, 10, -3, 0, 8, -1};
    std::vector<int> result4 = sortArrayAscending(test4);
    std::cout << "Test 4: ";
    for (int num : result4) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Test case 5: Array with duplicates
    std::vector<int> test5 = {5, 2, 8, 2, 9, 1, 5, 5};
    std::vector<int> result5 = sortArrayAscending(test5);
    std::cout << "Test 5: ";
    for (int num : result5) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
