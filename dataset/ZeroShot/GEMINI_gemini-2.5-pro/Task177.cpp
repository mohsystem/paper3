#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <limits>

class Task177 {
public:
    /**
     * Finds the maximum sum of a non-empty subsequence of the array `nums`
     * such that for every two consecutive integers in the subsequence, nums[i] and nums[j],
     * where i < j, the condition j - i <= k is satisfied.
     *
     * @param nums The input integer vector.
     * @param k The maximum distance between indices of consecutive elements in the subsequence.
     * @return The maximum subsequence sum.
     */
    int constrainedSubsetSum(std::vector<int>& nums, int k) {
        if (nums.empty()) {
            return 0;
        }

        int n = nums.size();
        // dp[i] will store the maximum subsequence sum ending at index i.
        std::vector<int> dp(n);
        // Deque will store indices `j` such that dp[j] are in decreasing order.
        std::deque<int> dq;
        int maxSum = std::numeric_limits<int>::min();

        for (int i = 0; i < n; ++i) {
            // 1. Remove indices from the front of the deque that are out of the window [i-k, i-1].
            if (!dq.empty() && dq.front() < i - k) {
                dq.pop_front();
            }

            // 2. Calculate dp[i]. The max sum of a valid subsequence ending before i
            //    is at the front of the deque.
            int maxPrev = 0;
            if (!dq.empty()) {
                maxPrev = dp[dq.front()];
            }
            
            // If maxPrev is negative, we start a new subsequence from nums[i].
            dp[i] = nums[i] + std::max(0, maxPrev);

            // 3. Maintain the decreasing property of dp values in the deque.
            //    If dp[i] is greater than dp values at the end of the deque,
            //    those smaller values can never be the maximum in any future window.
            while (!dq.empty() && dp[dq.back()] <= dp[i]) {
                dq.pop_back();
            }
            dq.push_back(i);

            // 4. Update the overall maximum sum found so far.
            maxSum = std::max(maxSum, dp[i]);
        }
        return maxSum;
    }
};

int main() {
    Task177 solution;

    // Test Case 1
    std::vector<int> nums1 = {10, 2, -10, 5, 20};
    int k1 = 2;
    std::cout << "Test Case 1: " << solution.constrainedSubsetSum(nums1, k1) << std::endl; // Expected: 37

    // Test Case 2
    std::vector<int> nums2 = {-1, -2, -3};
    int k2 = 1;
    std::cout << "Test Case 2: " << solution.constrainedSubsetSum(nums2, k2) << std::endl; // Expected: -1

    // Test Case 3
    std::vector<int> nums3 = {10, -2, -10, -5, 20};
    int k3 = 2;
    std::cout << "Test Case 3: " << solution.constrainedSubsetSum(nums3, k3) << std::endl; // Expected: 23
    
    // Test Case 4
    std::vector<int> nums4 = {-5, -6};
    int k4 = 2;
    std::cout << "Test Case 4: " << solution.constrainedSubsetSum(nums4, k4) << std::endl; // Expected: -5

    // Test Case 5
    std::vector<int> nums5 = {1, -5, -6, 2, 8, -4, 3};
    int k5 = 3;
    std::cout << "Test Case 5: " << solution.constrainedSubsetSum(nums5, k5) << std::endl; // Expected: 14

    return 0;
}