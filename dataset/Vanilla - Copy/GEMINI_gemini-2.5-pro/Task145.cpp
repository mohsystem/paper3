#include <iostream>
#include <vector>
#include <algorithm> // For std::max

/**
 * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 *
 * @param nums The input vector of integers.
 * @return The maximum sum of a contiguous subarray.
 */
int maxSubArraySum(const std::vector<int>& nums) {
    if (nums.empty()) {
        return 0; // The sum of an empty subarray is 0.
    }

    int maxSoFar = nums[0];
    int currentMax = nums[0];

    for (size_t i = 1; i < nums.size(); ++i) {
        currentMax = std::max(nums[i], currentMax + nums[i]);
        maxSoFar = std::max(maxSoFar, currentMax);
    }
    return maxSoFar;
}

// Helper function to print a vector for testing
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    std::cout << "Max sum for ";
    printVector(arr1);
    std::cout << " is: " << maxSubArraySum(arr1) << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {1};
    std::cout << "Max sum for ";
    printVector(arr2);
    std::cout << " is: " << maxSubArraySum(arr2) << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {5, 4, -1, 7, 8};
    std::cout << "Max sum for ";
    printVector(arr3);
    std::cout << " is: " << maxSubArraySum(arr3) << std::endl;

    // Test Case 4
    std::vector<int> arr4 = {-5, -1, -3};
    std::cout << "Max sum for ";
    printVector(arr4);
    std::cout << " is: " << maxSubArraySum(arr4) << std::endl;

    // Test Case 5
    std::vector<int> arr5 = {-2, -3, 4, -1, -2, 1, 5, -3};
    std::cout << "Max sum for ";
    printVector(arr5);
    std::cout << " is: " << maxSubArraySum(arr5) << std::endl;

    return 0;
}