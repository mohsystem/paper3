#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <limits>

/**
 * Calculates the maximum sum of a non-empty subsequence of the array
 * such that for every two consecutive integers in the subsequence,
 * nums[i] and nums[j] with i < j, the condition j - i <= k is satisfied.
 *
 * @param nums The input integer vector.
 * @param k The constraint for the distance between consecutive indices.
 * @return The maximum constrained subsequence sum.
 */
int constrainedSubsetSum(const std::vector<int>& nums, int k) {
    if (nums.empty()) {
        return 0;
    }
    int n = nums.size();
    std::vector<int> dp(n);
    std::deque<int> dq;
    int maxSum = std::numeric_limits<int>::min();

    for (int i = 0; i < n; ++i) {
        // Remove indices from the front that are out of the window [i-k, i-1]
        if (!dq.empty() && dq.front() < i - k) {
            dq.pop_front();
        }

        // Calculate dp[i]. The max sum from the previous valid window is at the front of the deque.
        int prevMax = 0;
        if (!dq.empty()) {
            prevMax = dp[dq.front()];
        }

        // If prevMax is positive, extend the subsequence. Otherwise, start a new one.
        dp[i] = nums[i] + std::max(0, prevMax);

        // Maintain the decreasing property of the deque (in terms of dp values)
        while (!dq.empty() && dp[dq.back()] <= dp[i]) {
            dq.pop_back();
        }
        dq.push_back(i);

        // Update the overall maximum sum found so far
        maxSum = std::max(maxSum, dp[i]);
    }
    return maxSum;
}

int main() {
    // Test Case 1
    std::vector<int> nums1 = {10, 2, -10, 5, 20};
    int k1 = 2;
    std::cout << "Test Case 1: " << constrainedSubsetSum(nums1, k1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {-1, -2, -3};
    int k2 = 1;
    std::cout << "Test Case 2: " << constrainedSubsetSum(nums2, k2) << std::endl;
    
    // Test Case 3
    std::vector<int> nums3 = {10, -2, -10, -5, 20};
    int k3 = 2;
    std::cout << "Test Case 3: " << constrainedSubsetSum(nums3, k3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {-8269, 3217, -4023, -4138, -683, 6455, -3621, 9242, 4015, -3790};
    int k4 = 1;
    std::cout << "Test Case 4: " << constrainedSubsetSum(nums4, k4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {1, -1, -2, -3, 5};
    int k5 = 2;
    std::cout << "Test Case 5: " << constrainedSubsetSum(nums5, k5) << std::endl;

    return 0;
}