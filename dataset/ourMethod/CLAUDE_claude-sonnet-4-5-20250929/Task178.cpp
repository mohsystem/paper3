
#include <vector>
#include <deque>
#include <climits>
#include <iostream>

// Function to find the shortest subarray with sum at least k
// Uses prefix sum and monotonic deque for optimal O(n) time complexity
// Security: All inputs validated, bounds checked, no dynamic memory issues
int shortestSubarray(const std::vector<int>& nums, int k) {
    // Input validation: check if nums is empty (though constraints say >= 1)
    if (nums.empty()) {
        return -1;
    }
    
    // Validate k is positive (constraint says >= 1)
    if (k <= 0) {
        return -1;
    }
    
    const size_t n = nums.size();
    
    // Check for potential overflow when creating prefix sum array
    // Max array size is 10^5, so prefix array size is bounded
    if (n > 100000) {
        return -1;
    }
    
    // Prefix sum array: prefixSum[i] = sum of nums[0..i-1]
    // prefixSum[0] = 0, prefixSum[i] = nums[0] + ... + nums[i-1]
    std::vector<long long> prefixSum(n + 1, 0);
    
    // Build prefix sum array with overflow protection
    // Each element can be at most 10^5 in absolute value
    for (size_t i = 0; i < n; ++i) {
        // Check for integer overflow: sum could exceed long long theoretically
        // With constraints: max sum = 10^5 * 10^5 = 10^10, fits in long long
        prefixSum[i + 1] = prefixSum[i] + nums[i];
    }
    
    int minLength = INT_MAX;
    
    // Monotonic deque to store indices of prefix sums
    // Maintains increasing order of prefix sum values
    std::deque<size_t> dq;
    
    // Process each prefix sum
    for (size_t i = 0; i <= n; ++i) {
        // Check if current prefix sum minus front of deque >= k
        // This means subarray from dq.front() to i-1 has sum >= k
        while (!dq.empty() && prefixSum[i] - prefixSum[dq.front()] >= k) {
            // Bounds check: ensure valid index access
            if (i > dq.front()) {
                int length = static_cast<int>(i - dq.front());
                if (length < minLength) {
                    minLength = length;
                }
            }
            dq.pop_front();
        }
        
        // Maintain monotonic property: remove indices with larger prefix sums
        // If prefixSum[i] <= prefixSum[dq.back()], remove dq.back()
        // This ensures we always consider the earliest position for minimum length
        while (!dq.empty() && prefixSum[i] <= prefixSum[dq.back()]) {
            dq.pop_back();
        }
        
        // Add current index to deque
        dq.push_back(i);
    }
    
    // Return result: -1 if no valid subarray found
    return (minLength == INT_MAX) ? -1 : minLength;
}

int main() {
    // Test case 1: Single element array
    std::vector<int> test1 = {1};
    std::cout << "Test 1: " << shortestSubarray(test1, 1) << " (expected: 1)" << std::endl;
    
    // Test case 2: No valid subarray
    std::vector<int> test2 = {1, 2};
    std::cout << "Test 2: " << shortestSubarray(test2, 4) << " (expected: -1)" << std::endl;
    
    // Test case 3: Array with negative numbers
    std::vector<int> test3 = {2, -1, 2};
    std::cout << "Test 3: " << shortestSubarray(test3, 3) << " (expected: 3)" << std::endl;
    
    // Test case 4: Large positive numbers
    std::vector<int> test4 = {1, 1, 1, 1, 1};
    std::cout << "Test 4: " << shortestSubarray(test4, 3) << " (expected: 3)" << std::endl;
    
    // Test case 5: Mix of positive and negative with target k
    std::vector<int> test5 = {84, -37, 32, 40, 95};
    std::cout << "Test 5: " << shortestSubarray(test5, 167) << " (expected: 3)" << std::endl;
    
    return 0;
}
