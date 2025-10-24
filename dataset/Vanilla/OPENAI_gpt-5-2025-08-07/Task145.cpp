#include <bits/stdc++.h>
using namespace std;

int maxSubArraySum(const vector<int>& nums) {
    if (nums.empty()) return 0;
    int current = nums[0], best = nums[0];
    for (size_t i = 1; i < nums.size(); ++i) {
        current = max(nums[i], current + nums[i]);
        best = max(best, current);
    }
    return best;
}

int main() {
    vector<vector<int>> tests = {
        {-2, 1, -3, 4, -1, 2, 1, -5, 4},
        {1},
        {5, 4, -1, 7, 8},
        {-1, -2, -3, -4},
        {0, 0, 0, 0}
    };
    for (const auto& t : tests) {
        cout << maxSubArraySum(t) << endl;
    }
    return 0;
}