#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <climits>

class Task177 {
public:
    int constrainedSubsetSum(std::vector<int>& nums, int k) {
        int n = nums.size();
        std::vector<int> dp(n);
        std::deque<int> dq;
        int maxSum = INT_MIN;

        for (int i = 0; i < n; ++i) {
            // Remove indices from the front that are out of the window [i-k, i-1]
            if (!dq.empty() && dq.front() < i - k) {
                dq.pop_front();
            }
            
            // Calculate dp[i]
            // The max sum from the previous k elements is at the front of the deque
            int prevMax = dq.empty() ? 0 : dp[dq.front()];
            
            // If prevMax is negative, we start a new subsequence from nums[i]
            dp[i] = nums[i] + std::max(0, prevMax);
            
            // Maintain the monotonically decreasing property of the deque based on dp values
            while (!dq.empty() && dp[i] >= dp[dq.back()]) {
                dq.pop_back();
            }
            dq.push_back(i);

            maxSum = std::max(maxSum, dp[i]);
        }
        return maxSum;
    }
};

int main() {
    Task177 solver;

    // Test Case 1
    std::vector<int> nums1 = {10, 2, -10, 5, 20};
    std::cout << "Test Case 1: " << solver.constrainedSubsetSum(nums1, 2) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {-1, -2, -3};
    std::cout << "Test Case 2: " << solver.constrainedSubsetSum(nums2, 1) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {10, -2, -10, -5, 20};
    std::cout << "Test Case 3: " << solver.constrainedSubsetSum(nums3, 2) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {1, -2, 3, 4, -5, 6};
    std::cout << "Test Case 4: " << solver.constrainedSubsetSum(nums4, 3) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {-5, -4, -3, -2, -1};
    std::cout << "Test Case 5: " << solver.constrainedSubsetSum(nums5, 5) << std::endl;

    return 0;
}