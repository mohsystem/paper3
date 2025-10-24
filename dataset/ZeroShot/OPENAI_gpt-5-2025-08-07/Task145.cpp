#include <iostream>
#include <vector>
#include <algorithm>

long long maxSubarraySum(const std::vector<int>& nums) {
    if (nums.empty()) {
        return 0LL;
    }
    long long current = nums[0];
    long long best = nums[0];
    for (size_t i = 1; i < nums.size(); ++i) {
        long long v = nums[i];
        current = std::max(v, current + v);
        best = std::max(best, current);
    }
    return best;
}

int main() {
    std::vector<std::vector<int>> tests = {
        std::vector<int>{-2, 1, -3, 4, -1, 2, 1, -5, 4},
        std::vector<int>{1, 2, 3, 4},
        std::vector<int>{-1, -2, -3},
        std::vector<int>{5, -2, 3, -1, 2},
        std::vector<int>{} // empty array
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " input: [";
        for (size_t j = 0; j < tests[i].size(); ++j) {
            std::cout << tests[i][j] << (j + 1 < tests[i].size() ? ", " : "");
        }
        std::cout << "] -> Max Sum: " << maxSubarraySum(tests[i]) << "\n";
    }
    return 0;
}