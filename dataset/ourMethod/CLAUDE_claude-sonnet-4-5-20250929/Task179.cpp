
#include <vector>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <climits>

// Sliding window maximum using monotonic deque
// Time: O(n), Space: O(k)
std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
    // Input validation: check if nums is empty
    if (nums.empty()) {
        throw std::invalid_argument("Input array cannot be empty");
    }
    
    // Input validation: check k bounds (per constraints: 1 <= k <= nums.length)
    if (k < 1 || k > static_cast<int>(nums.size())) {
        throw std::invalid_argument("Invalid window size k");
    }
    
    // Prevent integer overflow when calculating result size
    size_t result_size = nums.size() - k + 1;
    if (result_size > INT_MAX) {
        throw std::overflow_error("Result size exceeds maximum allowed");
    }
    
    std::vector<int> result;
    result.reserve(result_size); // Pre-allocate to avoid reallocations
    
    // Deque stores indices in decreasing order of their values
    std::deque<int> dq;
    
    for (size_t i = 0; i < nums.size(); ++i) {
        // Bounds check: ensure index is valid
        if (i >= nums.size()) {
            throw std::out_of_range("Index out of bounds");
        }
        
        // Remove indices outside current window
        while (!dq.empty() && dq.front() <= static_cast<int>(i) - k) {
            dq.pop_front();
        }
        
        // Remove indices whose values are smaller than current value
        // Maintain decreasing order in deque
        while (!dq.empty() && nums[dq.back()] <= nums[i]) {
            dq.pop_back();
        }
        
        dq.push_back(static_cast<int>(i));
        
        // Start recording results when first window is complete
        if (i >= static_cast<size_t>(k - 1)) {
            result.push_back(nums[dq.front()]);
        }
    }
    
    return result;
}

int main() {
    // Test case 1: Example from problem
    std::vector<int> test1 = {1, 3, -1, -3, 5, 3, 6, 7};
    std::vector<int> result1 = maxSlidingWindow(test1, 3);
    std::cout << "Test 1: ";
    for (int val : result1) std::cout << val << " ";
    std::cout << std::endl; // Expected: 3 3 5 5 6 7
    
    // Test case 2: Single element
    std::vector<int> test2 = {1};
    std::vector<int> result2 = maxSlidingWindow(test2, 1);
    std::cout << "Test 2: ";
    for (int val : result2) std::cout << val << " ";
    std::cout << std::endl; // Expected: 1
    
    // Test case 3: Window size equals array size
    std::vector<int> test3 = {1, -1, 5, 3};
    std::vector<int> result3 = maxSlidingWindow(test3, 4);
    std::cout << "Test 3: ";
    for (int val : result3) std::cout << val << " ";
    std::cout << std::endl; // Expected: 5
    
    // Test case 4: All negative numbers
    std::vector<int> test4 = {-7, -8, -3, -1, -4};
    std::vector<int> result4 = maxSlidingWindow(test4, 2);
    std::cout << "Test 4: ";
    for (int val : result4) std::cout << val << " ";
    std::cout << std::endl; // Expected: -7 -3 -1 -1
    
    // Test case 5: Decreasing sequence
    std::vector<int> test5 = {9, 7, 5, 3, 1};
    std::vector<int> result5 = maxSlidingWindow(test5, 3);
    std::cout << "Test 5: ";
    for (int val : result5) std::cout << val << " ";
    std::cout << std::endl; // Expected: 9 7 5
    
    return 0;
}
