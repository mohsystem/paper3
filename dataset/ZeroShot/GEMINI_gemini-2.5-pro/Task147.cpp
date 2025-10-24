#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

/**
 * Finds the kth largest element in a vector of integers using std::nth_element.
 * Note: This function partially sorts the input vector in-place.
 *
 * @param nums The input vector of integers. Will be modified.
 * @param k    The 'k' value, representing the desired largest element rank.
 * @return The kth largest element.
 * @throws std::invalid_argument if the input is invalid.
 */
int findKthLargest(std::vector<int>& nums, int k) {
    // 1. Input Validation
    if (nums.empty() || k < 1 || static_cast<size_t>(k) > nums.size()) {
        throw std::invalid_argument("Invalid input: vector is empty or k is out of bounds.");
    }

    // 2. Use std::nth_element for an average O(N) solution.
    // It rearranges the vector such that the element at the specified position
    // is the one that would be in that position in a fully sorted vector.
    // The kth largest element is at index (size - k) in a 0-indexed sorted vector.
    size_t nth_index = nums.size() - k;
    std::nth_element(nums.begin(), nums.begin() + nth_index, nums.end());
    
    // 3. The element at this position is our answer.
    return nums[nth_index];
}

void run_test(const std::string& name, std::vector<int> nums, int k) {
    std::cout << name << ": Array = [";
    for(size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << (i == nums.size() - 1 ? "" : ", ");
    }
    std::cout << "], k = " << k;
    try {
        int result = findKthLargest(nums, k);
        std::cout << " -> Result: " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "\nCaught expected exception -> " << e.what() << std::endl;
    }
}

int main() {
    // Test Case 1
    run_test("Test Case 1", {3, 2, 1, 5, 6, 4}, 2);

    // Test Case 2
    run_test("Test Case 2", {3, 2, 3, 1, 2, 4, 5, 5, 6}, 4);

    // Test Case 3
    run_test("Test Case 3", {1}, 1);

    // Test Case 4
    run_test("Test Case 4", {7, 6, 5, 4, 3, 2, 1}, 7);

    // Test Case 5
    run_test("Test Case 5", {99, 99}, 1);

    // Invalid Input Test
    run_test("Invalid Input Test", {}, 1);
    
    return 0;
}