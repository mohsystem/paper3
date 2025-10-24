#include <iostream>
#include <vector>
#include <deque>
#include <numeric>
#include <algorithm>

class Task178 {
public:
    int shortestSubarray(std::vector<int>& nums, int k) {
        int n = nums.size();
        std::vector<long long> prefixSums(n + 1, 0);
        for (int i = 0; i < n; ++i) {
            prefixSums[i + 1] = prefixSums[i] + nums[i];
        }

        int minLength = n + 1;
        std::deque<int> dq;

        for (int i = 0; i <= n; ++i) {
            // Check for valid subarrays ending at i-1
            while (!dq.empty() && prefixSums[i] - prefixSums[dq.front()] >= k) {
                minLength = std::min(minLength, i - dq.front());
                dq.pop_front();
            }

            // Maintain monotonic property of the deque (increasing prefix sums)
            while (!dq.empty() && prefixSums[i] <= prefixSums[dq.back()]) {
                dq.pop_back();
            }

            dq.push_back(i);
        }

        return minLength == n + 1 ? -1 : minLength;
    }
};

int main() {
    Task178 solver;

    // Test Case 1
    std::vector<int> nums1 = {1};
    int k1 = 1;
    std::cout << "Test Case 1: " << solver.shortestSubarray(nums1, k1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {1, 2};
    int k2 = 4;
    std::cout << "Test Case 2: " << solver.shortestSubarray(nums2, k2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {2, -1, 2};
    int k3 = 3;
    std::cout << "Test Case 3: " << solver.shortestSubarray(nums3, k3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {84, -37, 32, 40, 95};
    int k4 = 167;
    std::cout << "Test Case 4: " << solver.shortestSubarray(nums4, k4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {-28, 81, -20, 28, -29};
    int k5 = 89;
    std::cout << "Test Case 5: " << solver.shortestSubarray(nums5, k5) << std::endl;
    
    return 0;
}