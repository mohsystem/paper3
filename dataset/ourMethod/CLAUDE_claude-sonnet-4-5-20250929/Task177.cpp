
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
#include <deque>

// Dynamic programming solution with monotonic deque optimization
// Security: All inputs validated, bounds checked, no undefined behavior
int constrainedSubsetSum(std::vector<int>& nums, int k) {
    // Input validation per Rules#1 and Rules#6
    if (nums.empty() || k < 1) {
        return INT_MIN; // Invalid input
    }
    
    int n = static_cast<int>(nums.size());
    
    // Validate k against array size (Rules#6)
    if (k > n) {
        k = n;
    }
    
    // dp[i] represents max sum ending at index i
    // Using RAII with std::vector for automatic memory management (Rules#1)
    std::vector<int> dp(n);
    
    // Monotonic deque to maintain max in sliding window
    // Stores indices, maintaining decreasing order of dp values
    std::deque<int> dq;
    
    int maxSum = INT_MIN;
    
    // Process each element with bounds checking (Rules#3)
    for (int i = 0; i < n; i++) {
        // Remove indices outside the k-window (Rules#3: bounds check)
        while (!dq.empty() && dq.front() < i - k) {
            dq.pop_front();
        }
        
        // Current dp value: either start new or extend from max in window
        if (dq.empty()) {
            dp[i] = nums[i];
        } else {
            // Bounds check before array access (Rules#3)
            int maxPrev = dp[dq.front()];
            dp[i] = std::max(nums[i], nums[i] + maxPrev);
        }
        
        // Maintain monotonic decreasing deque
        // Remove elements from back that are smaller than current
        while (!dq.empty() && dp[dq.back()] <= dp[i]) {
            dq.pop_back();
        }
        
        dq.push_back(i);
        
        // Track global maximum
        maxSum = std::max(maxSum, dp[i]);
    }
    
    return maxSum;
}

int main() {
    // Test case 1
    std::vector<int> nums1 = {10, 2, -10, 5, 20};
    int k1 = 2;
    std::cout << "Test 1: " << constrainedSubsetSum(nums1, k1) << " (expected: 37)" << std::endl;
    
    // Test case 2
    std::vector<int> nums2 = {-1, -2, -3};
    int k2 = 1;
    std::cout << "Test 2: " << constrainedSubsetSum(nums2, k2) << " (expected: -1)" << std::endl;
    
    // Test case 3
    std::vector<int> nums3 = {10, -2, -10, -5, 20};
    int k3 = 2;
    std::cout << "Test 3: " << constrainedSubsetSum(nums3, k3) << " (expected: 23)" << std::endl;
    
    // Test case 4: Single element
    std::vector<int> nums4 = {5};
    int k4 = 1;
    std::cout << "Test 4: " << constrainedSubsetSum(nums4, k4) << " (expected: 5)" << std::endl;
    
    // Test case 5: All positive
    std::vector<int> nums5 = {1, 2, 3, 4, 5};
    int k5 = 2;
    std::cout << "Test 5: " << constrainedSubsetSum(nums5, k5) << " (expected: 15)" << std::endl;
    
    return 0;
}
