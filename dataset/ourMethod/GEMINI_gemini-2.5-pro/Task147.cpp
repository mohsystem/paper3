#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

/**
 * Finds the kth largest element in a vector of integers.
 * This implementation sorts a copy of the vector and picks the element at the correct index.
 *
 * @param nums The input vector of integers (passed by const reference for efficiency).
 * @param k    The rank of the element to find (1-based index).
 * @return The kth largest element.
 * @throws std::invalid_argument if the input is invalid (e.g., empty vector, k is out of bounds).
 */
int findKthLargest(const std::vector<int>& nums, int k) {
    if (nums.empty() || k <= 0 || static_cast<size_t>(k) > nums.size()) {
        throw std::invalid_argument("Invalid input: vector must not be empty, and k must be within the bounds [1, vector.size()].");
    }

    // Create a copy to avoid modifying the original vector.
    std::vector<int> sorted_nums = nums;
    
    // Sort the vector in ascending order.
    std::sort(sorted_nums.begin(), sorted_nums.end());
    
    // The kth largest element is at index size() - k.
    return sorted_nums[sorted_nums.size() - k];
}

void run_test_case(int test_num, const std::vector<int>& nums, int k, int expected) {
    std::cout << "Test Case " << test_num << ":" << std::endl;
    std::cout << "Input vector: { ";
    for (size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << (i == nums.size() - 1 ? "" : ", ");
    }
    std::cout << " }" << std::endl;
    std::cout << "k: " << k << std::endl;
    
    try {
        int result = findKthLargest(nums, k);
        std::cout << "Result: " << result << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        if (result == expected) {
            std::cout << "Status: PASSED" << std::endl;
        } else {
            std::cout << "Status: FAILED" << std::endl;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        std::cout << "Status: FAILED (exception not expected)" << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

int main() {
    run_test_case(1, {3, 2, 1, 5, 6, 4}, 2, 5);
    run_test_case(2, {3, 2, 3, 1, 2, 4, 5, 5, 6}, 4, 4);
    run_test_case(3, {1}, 1, 1);
    run_test_case(4, {99, 99}, 1, 99);
    run_test_case(5, {-1, -1, -2, -5}, 3, -2);
    
    // Additional test case for invalid input
    std::cout << "Test Case 6: Invalid k (k=4 for vector of size 3)" << std::endl;
    try {
        findKthLargest({1, 2, 3}, 4);
        std::cout << "Status: FAILED (exception was expected)" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
        std::cout << "Status: PASSED" << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    return 0;
}