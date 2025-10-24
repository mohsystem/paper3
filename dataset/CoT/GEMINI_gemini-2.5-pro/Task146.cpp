#include <iostream>
#include <vector>
#include <numeric> // Required for std::accumulate

/**
 * @brief Finds the missing number in a vector containing n-1 unique integers
 * from the range [1, n].
 * 
 * @param nums The input vector of unique integers.
 * @return The missing integer.
 */
int findMissingNumber(const std::vector<int>& nums) {
    // 'n' is the expected size of the complete sequence
    size_t n = nums.size() + 1;
    
    // Use long long to prevent potential integer overflow for large n
    long long expectedSum = (long long)n * (n + 1) / 2;
    
    long long actualSum = 0;
    // Using std::accumulate to sum up vector elements
    // The third argument 0LL specifies the initial sum is a long long
    actualSum = std::accumulate(nums.begin(), nums.end(), 0LL);
    
    // The difference is the missing number
    return static_cast<int>(expectedSum - actualSum);
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {1, 2, 4, 5};
    std::cout << "Test Case 1: Input: [1, 2, 4, 5], Missing Number: " << findMissingNumber(arr1) << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {2, 3, 1, 5};
    std::cout << "Test Case 2: Input: [2, 3, 1, 5], Missing Number: " << findMissingNumber(arr2) << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {1};
    std::cout << "Test Case 3: Input: [1], Missing Number: " << findMissingNumber(arr3) << std::endl;

    // Test Case 4
    std::vector<int> arr4 = {2};
    std::cout << "Test Case 4: Input: [2], Missing Number: " << findMissingNumber(arr4) << std::endl;

    // Test Case 5 (Edge case: empty vector)
    std::vector<int> arr5 = {};
    std::cout << "Test Case 5: Input: [], Missing Number: " << findMissingNumber(arr5) << std::endl;

    return 0;
}