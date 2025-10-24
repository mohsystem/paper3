#include <bits/stdc++.h>
using namespace std;

long long constrainedSubsequenceSum(const vector<int>& nums, int k) {
    if (nums.empty()) {
        throw invalid_argument("nums must be non-empty");
    }
    if (k < 1 || k > static_cast<int>(nums.size())) {
        throw invalid_argument("k must satisfy 1 <= k <= nums.size()");
    }

    int n = static_cast<int>(nums.size());
    vector<long long> dp(n, 0);
    deque<int> dq;
    long long ans = LLONG_MIN;

    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && dq.front() < i - k) dq.pop_front();
        long long bestPrev = 0;
        if (!dq.empty()) bestPrev = max(0LL, dp[dq.front()]);
        dp[i] = static_cast<long long>(nums[i]) + bestPrev;
        ans = max(ans, dp[i]);
        while (!dq.empty() && dp[dq.back()] <= dp[i]) dq.pop_back();
        dq.push_back(i);
    }
    return ans;
}

static void runTest(const vector<int>& nums, int k) {
    try {
        long long res = constrainedSubsequenceSum(nums, k);
        cout << "Result: " << res << "\n";
    } catch (const exception& ex) {
        cout << "Error: " << ex.what() << "\n";
    }
}

int main() {
    // 5 test cases
    runTest({10, 2, -10, 5, 20}, 2);        // Expected 37
    runTest({-1, -2, -3}, 1);               // Expected -1
    runTest({10, -2, -10, -5, 20}, 2);      // Expected 23
    runTest({5}, 1);                        // Expected 5
    runTest({1, -1, 1, -1, 1}, 4);          // Expected 3
    return 0;
}