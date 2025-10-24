#include <iostream>
#include <vector>
#include <deque>
#include <numeric>
#include <algorithm>
#include <climits>

class Task178 {
public:
    int shortestSubarray(const std::vector<int>& nums, int k) {
        int n = nums.size();
        // Use long long for prefix sums to avoid integer overflow
        std::vector<long long> prefixSums(n + 1, 0);
        for (int i = 0; i < n; ++i) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }

        int minLength = INT_MAX;
        // Deque stores indices of the prefixSums vector
        std::deque<int> dq;

        for (int i = 0; i <= n; ++i) {
            // Condition 1: Find a valid subarray
            while (!dq.empty() && prefixSums[i] - prefixSums[dq.front()] >= k) {
                minLength = std::min(minLength, i - dq.front());
                dq.pop_front();
            }

            // Condition 2: Maintain monotonic property (increasing prefix sums)
            while (!dq.empty() && prefixSums[i] <= prefixSums[dq.back()]) {
                dq.pop_back();
            }

            dq.push_back(i);
        }

        return minLength == INT_MAX ? -1 : minLength;
    }
};

int main() {
    Task178 solution;

    // Test Case 1
    std::vector<int> nums1 = {1};
    std::cout << "Test Case 1: " << solution.shortestSubarray(nums1, 1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {1, 2};
    std::cout << "Test Case 2: " << solution.shortestSubarray(nums2, 4) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {2, -1, 2};
    std::cout << "Test Case 3: " << solution.shortestSubarray(nums3, 3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {84, -37, 32, 40, 95};
    std::cout << "Test Case 4: " << solution.shortestSubarray(nums4, 167) << std::endl;
    
    // Test Case 5
    std::vector<int> nums5 = {-28, 81, -20, 28, -29};
    std::cout << "Test Case 5: " << solution.shortestSubarray(nums5, 89) << std::endl;

    return 0;
}