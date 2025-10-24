
#include <iostream>
#include <vector>
#include <limits>
#include <stdexcept>

// Merges two sorted arrays into a single sorted array
// Security: Validates input sizes to prevent integer overflow and excessive memory allocation
// Returns empty vector if either input is empty
std::vector<int> mergeSortedArrays(const std::vector<int>& arr1, const std::vector<int>& arr2) {
    // Input validation: check for empty arrays
    if (arr1.empty() && arr2.empty()) {
        return std::vector<int>();
    }
    
    size_t size1 = arr1.size();
    size_t size2 = arr2.size();
    
    // Security: Prevent integer overflow when calculating result size
    // Maximum safe size to prevent excessive memory allocation (100MB limit)
    const size_t MAX_SAFE_SIZE = 100000000 / sizeof(int);
    
    if (size1 > MAX_SAFE_SIZE || size2 > MAX_SAFE_SIZE) {
        throw std::invalid_argument("Input arrays too large");
    }
    
    // Check for overflow in addition
    if (size1 > SIZE_MAX - size2) {
        throw std::overflow_error("Combined array size would overflow");
    }
    
    size_t totalSize = size1 + size2;
    if (totalSize > MAX_SAFE_SIZE) {
        throw std::invalid_argument("Combined array size exceeds maximum safe size");
    }
    
    // Reserve exact space needed (RAII ensures cleanup)
    std::vector<int> result;
    result.reserve(totalSize);
    
    // Two-pointer merge algorithm with bounds checking
    size_t i = 0;
    size_t j = 0;
    
    // Merge while both arrays have elements
    // Bounds are implicitly checked by comparison with size1/size2
    while (i < size1 && j < size2) {
        if (arr1[i] <= arr2[j]) {
            result.push_back(arr1[i]);
            ++i;
        } else {
            result.push_back(arr2[j]);
            ++j;
        }
    }
    
    // Copy remaining elements from arr1 (if any)
    while (i < size1) {
        result.push_back(arr1[i]);
        ++i;
    }
    
    // Copy remaining elements from arr2 (if any)
    while (j < size2) {
        result.push_back(arr2[j]);
        ++j;
    }
    
    return result;
}

// Helper function to print vector
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    try {
        // Test case 1: Normal case with positive integers
        std::vector<int> arr1_1 = {1, 3, 5, 7};
        std::vector<int> arr2_1 = {2, 4, 6, 8};
        std::vector<int> result1 = mergeSortedArrays(arr1_1, arr2_1);
        std::cout << "Test 1: ";
        printVector(result1);
        
        // Test case 2: One empty array
        std::vector<int> arr1_2 = {1, 2, 3};
        std::vector<int> arr2_2 = {};
        std::vector<int> result2 = mergeSortedArrays(arr1_2, arr2_2);
        std::cout << "Test 2: ";
        printVector(result2);
        
        // Test case 3: Arrays with negative and positive integers
        std::vector<int> arr1_3 = {-5, -1, 3, 10};
        std::vector<int> arr2_3 = {-3, 0, 5, 12};
        std::vector<int> result3 = mergeSortedArrays(arr1_3, arr2_3);
        std::cout << "Test 3: ";
        printVector(result3);
        
        // Test case 4: Arrays with duplicate values
        std::vector<int> arr1_4 = {1, 3, 3, 7};
        std::vector<int> arr2_4 = {3, 5, 7, 9};
        std::vector<int> result4 = mergeSortedArrays(arr1_4, arr2_4);
        std::cout << "Test 4: ";
        printVector(result4);
        
        // Test case 5: Both empty arrays
        std::vector<int> arr1_5 = {};
        std::vector<int> arr2_5 = {};
        std::vector<int> result5 = mergeSortedArrays(arr1_5, arr2_5);
        std::cout << "Test 5: ";
        printVector(result5);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
