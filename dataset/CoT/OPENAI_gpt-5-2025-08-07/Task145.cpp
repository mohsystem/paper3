// Task145 - C++ implementation
// Chain-of-Through process:
// 1) Problem: Max sum of a contiguous subarray.
// 2) Security: Use 64-bit accumulation; handle empty input.
// 3) Secure coding: No unsafe memory; no external input.
// 4) Code review: All-negative safe; empty returns 0.
// 5) Final secure output.

#include <iostream>
#include <vector>
#include <cstddef>
#include <limits>

long long maxSubArray(const std::vector<int>& nums) {
    if (nums.empty()) {
        return 0LL;
    }
    long long best = std::numeric_limits<long long>::min();
    long long curr = 0LL;
    for (int x : nums) {
        long long lx = static_cast<long long>(x);
        curr = std::max(lx, curr + lx);
        if (curr > best) {
            best = curr;
        }
    }
    return best;
}

int main() {
    // 5 test cases
    std::cout << maxSubArray({1, -3, 2, 1, -1}) << "\n";        // 3
    std::cout << maxSubArray({-2, -3, -1, -4}) << "\n";         // -1
    std::cout << maxSubArray({5}) << "\n";                      // 5
    std::cout << maxSubArray({}) << "\n";                       // 0
    std::cout << maxSubArray({4, -1, 2, 1, -5, 4}) << "\n";     // 6
    return 0;
}