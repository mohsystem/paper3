#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

/**
 * Finds the kth largest element in a vector of integers.
 * This function sorts the vector and picks the element from the end.
 *
 * @param nums The input vector of integers. Passed by value to avoid modifying the original.
 * @param k The position 'k' of the largest element to find (1-based index).
 * @return The integer value of the kth largest element.
 * @throws std::invalid_argument if the vector is empty or if k is out of bounds.
 */
int findKthLargest(std::vector<int> nums, int k) {
    // Security: Validate inputs to prevent out-of-bounds access and handle invalid arguments.
    if (nums.empty() || k <= 0 || k > nums.size()) {
        throw std::invalid_argument("Invalid input: Vector must not be empty and k must be between 1 and the vector's size.");
    }

    // Sort the vector in ascending order. Time complexity is O(N log N).
    // The vector 'nums' is a copy, so the original remains unchanged.
    std::sort(nums.begin(), nums.end());

    // The kth largest element is at index (size - k) in a 0-indexed vector
    // sorted in ascending order.
    return nums[nums.size() - k];
}

void runTestCase(const std::string& name, std::vector<int> nums, int k) {
    std::cout << name << ": Array = [";
    for (size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << (i == nums.size() - 1 ? "" : ", ");
    }
    std::cout << "], k = " << k << std::endl;
    
    try {
        int result = findKthLargest(nums, k);
        std::cout << "Result: " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Result: Caught expected exception -> " << e.what() << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

int main() {
    // Test Case 1: General case
    runTestCase("Test Case 1", {3, 2, 1, 5, 6, 4}, 2); // Expected: 5

    // Test Case 2: With duplicates
    runTestCase("Test Case 2", {3, 2, 3, 1, 2, 4, 5, 5, 6}, 4); // Expected: 4

    // Test Case 3: k = 1 (the largest element)
    runTestCase("Test Case 3", {7, 6, 5, 4, 3, 2, 1}, 1); // Expected: 7

    // Test Case 4: k = n (the smallest element)
    runTestCase("Test Case 4", {7, 6, 5, 4, 3, 2, 1}, 7); // Expected: 1

    // Test Case 5: Invalid k (k > size of vector)
    runTestCase("Test Case 5", {1, 2}, 3); // Expected: Exception

    return 0;
}