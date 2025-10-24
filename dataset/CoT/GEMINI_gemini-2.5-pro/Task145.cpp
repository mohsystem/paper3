#include <iostream>
#include <vector>
#include <algorithm>

/**
 * @brief Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 * 
 * @param arr The input vector of integers.
 * @return The maximum sum of a contiguous subarray. Returns 0 if the vector is empty.
 */
long long findMaxSubarraySum(const std::vector<int>& arr) {
    // Handle empty vector case.
    if (arr.empty()) {
        return 0;
    }

    // Use long long to prevent integer overflow with large sums.
    long long maxSoFar = arr[0];
    long long currentMax = arr[0];

    for (size_t i = 1; i < arr.size(); ++i) {
        // Decide whether to extend the existing subarray or start a new one.
        // Cast arr[i] to long long to ensure the addition is done in 64-bit.
        currentMax = std::max((long long)arr[i], currentMax + arr[i]);
        // Update the overall maximum sum found so far.
        maxSoFar = std::max(maxSoFar, currentMax);
    }

    return maxSoFar;
}

// Helper function to print a vector
void printVector(const std::string& name, const std::vector<int>& v) {
    std::cout << name << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i] << (i == v.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}


int main() {
    // Test Case 1: Mixed positive and negative numbers
    std::vector<int> test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    printVector("Array", test1);
    std::cout << "Maximum subarray sum: " << findMaxSubarraySum(test1) << std::endl << std::endl; // Expected: 6

    // Test Case 2: All positive numbers
    std::vector<int> test2 = {1, 2, 3, 4, 5};
    printVector("Array", test2);
    std::cout << "Maximum subarray sum: " << findMaxSubarraySum(test2) << std::endl << std::endl; // Expected: 15

    // Test Case 3: All negative numbers
    std::vector<int> test3 = {-2, -3, -4, -1, -2, -1, -5, -3};
    printVector("Array", test3);
    std::cout << "Maximum subarray sum: " << findMaxSubarraySum(test3) << std::endl << std::endl; // Expected: -1
    
    // Test Case 4: Single element array
    std::vector<int> test4 = {5};
    printVector("Array", test4);
    std::cout << "Maximum subarray sum: " << findMaxSubarraySum(test4) << std::endl << std::endl; // Expected: 5
    
    // Test Case 5: Empty array
    std::vector<int> test5 = {};
    printVector("Array", test5);
    std::cout << "Maximum subarray sum: " << findMaxSubarraySum(test5) << std::endl << std::endl; // Expected: 0

    return 0;
}