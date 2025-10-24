#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <limits>

class Task177 {
public:
    static int constrainedSubsetSum(std::vector<int>& nums, int k) {
        int n = nums.size();
        // dp[i] will store the maximum sum of a subsequence ending at index i
        std::vector<int> dp(n);
        // A deque to store indices of dp elements, maintaining them in decreasing order of their values
        std::deque<int> dq;
        int maxSum = std::numeric_limits<int>::min();

        for (int i = 0; i < n; ++i) {
            // Remove indices from the front of the deque that are out of the k-sized window
            while (!dq.empty() && dq.front() < i - k) {
                dq.pop_front();
            }

            // The max sum of a subsequence ending in the previous window is at the front of the deque
            int prevMax = dq.empty() ? 0 : dp[dq.front()];

            // Calculate dp[i]: it's nums[i] plus the max sum from the previous valid subsequence.
            // If the previous max sum is negative, we start a new subsequence from nums[i].
            dp[i] = nums[i] + std::max(0, prevMax);

            // Maintain the deque in decreasing order of dp values
            // Remove elements from the back of the deque that are smaller than the current dp[i]
            while (!dq.empty() && dp[dq.back()] <= dp[i]) {
                dq.pop_back();
            }

            // Add the current index to the deque
            dq.push_back(i);

            // Update the overall maximum sum found so far
            maxSum = std::max(maxSum, dp[i]);
        }

        return maxSum;
    }
};

int main() {
    // Test Case 1
    std::vector<int> nums1 = {10, 2, -10, 5, 20};
    int k1 = 2;
    std::cout << "Test Case 1: " << Task177::constrainedSubsetSum(nums1, k1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {-1, -2, -3};
    int k2 = 1;
    std::cout << "Test Case 2: " << Task177::constrainedSubsetSum(nums2, k2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {10, -2, -10, -5, 20};
    int k3 = 2;
    std::cout << "Test Case 3: " << Task177::constrainedSubsetSum(nums3, k3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {-5, -4, -3, -2, -1};
    int k4 = 3;
    std::cout << "Test Case 4: " << Task177::constrainedSubsetSum(nums4, k4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {1, -5, 2, -6, 3, -7, 4};
    int k5 = 2;
    std::cout << "Test Case 5: " << Task177::constrainedSubsetSum(nums5, k5) << std::endl;

    return 0;
}