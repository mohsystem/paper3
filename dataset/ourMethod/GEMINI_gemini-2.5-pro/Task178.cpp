#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>

/**
 * @brief Finds the length of the shortest non-empty subarray with a sum of at least k.
 * 
 * @param nums The input integer vector.
 * @param k The target sum.
 * @return The length of the shortest subarray, or -1 if no such subarray exists.
 */
int shortestSubarray(const std::vector<int>& nums, int k) {
    int n = nums.size();
    std::vector<long long> prefixSums(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    int minLength = n + 1;
    // Deque stores indices of the prefixSums vector.
    std::deque<int> dq;

    for (int j = 0; j <= n; ++j) {
        // Condition 1: Find a valid subarray.
        // If prefixSums[j] - prefixSums[dq.front()] >= k, we found a subarray.
        // We want the shortest, so we check from the left of the deque.
        while (!dq.empty() && prefixSums[j] - prefixSums[dq.front()] >= k) {
            minLength = std::min(minLength, j - dq.front());
            dq.pop_front();
        }

        // Condition 2: Maintain a monotonically increasing prefixSums in the deque.
        // If prefixSums[j] <= prefixSums[dq.back()], the last element is no longer optimal.
        // A future subarray starting at index j would be shorter and have a smaller or equal prefix sum.
        while (!dq.empty() && prefixSums[j] <= prefixSums[dq.back()]) {
            dq.pop_back();
        }

        dq.push_back(j);
    }

    return minLength == n + 1 ? -1 : minLength;
}

int main() {
    // Test case 1: Basic case
    std::vector<int> nums1 = {1};
    int k1 = 1;
    std::cout << "Test 1: " << shortestSubarray(nums1, k1) << std::endl; // Expected: 1

    // Test case 2: No such subarray
    std::vector<int> nums2 = {1, 2};
    int k2 = 4;
    std::cout << "Test 2: " << shortestSubarray(nums2, k2) << std::endl; // Expected: -1

    // Test case 3: With negative numbers
    std::vector<int> nums3 = {2, -1, 2};
    int k3 = 3;
    std::cout << "Test 3: " << shortestSubarray(nums3, k3) << std::endl; // Expected: 3

    // Test case 4: More complex case
    std::vector<int> nums4 = {84, -37, 32, 40, 95};
    int k4 = 167;
    std::cout << "Test 4: " << shortestSubarray(nums4, k4) << std::endl; // Expected: 3

    // Test case 5: All negative numbers
    std::vector<int> nums5 = {-1, -2, -3};
    int k5 = 1;
    std::cout << "Test 5: " << shortestSubarray(nums5, k5) << std::endl; // Expected: -1

    return 0;
}