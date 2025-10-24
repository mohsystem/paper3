#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>

/**
 * @brief Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 * 
 * @param nums The input vector of integers.
 * @return The maximum sum of a contiguous subarray.
 * @throws std::invalid_argument if the input vector is empty.
 */
int findMaxSubarraySum(const std::vector<int>& nums) {
    if (nums.empty()) {
        throw std::invalid_argument("Input array cannot be empty.");
    }

    int maxSoFar = nums[0];
    int currentMax = nums[0];

    for (size_t i = 1; i < nums.size(); ++i) {
        currentMax = std::max(nums[i], currentMax + nums[i]);
        maxSoFar = std::max(maxSoFar, currentMax);
    }

    return maxSoFar;
}

void run_test_case(const std::string& name, const std::vector<int>& test_vec) {
    std::cout << name << ": { ";
    for (size_t i = 0; i < test_vec.size(); ++i) {
        std::cout << test_vec[i] << (i == test_vec.size() - 1 ? "" : ", ");
    }
    std::cout << " }" << std::endl;
    try {
        int result = findMaxSubarraySum(test_vec);
        std::cout << "Max sum: " << result << std::endl << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl << std::endl;
    }
}

int main() {
    // Test Case 1: Mixed positive and negative numbers
    run_test_case("Test Case 1", {-2, 1, -3, 4, -1, 2, 1, -5, 4});

    // Test Case 2: Mostly positive numbers
    run_test_case("Test Case 2", {5, 4, -1, 7, 8});

    // Test Case 3: All negative numbers
    run_test_case("Test Case 3", {-2, -3, -1, -5});

    // Test Case 4: Single element array
    run_test_case("Test Case 4", {5});

    // Test Case 5: Another mixed array
    run_test_case("Test Case 5", {8, -19, 5, -4, 20});

    return 0;
}